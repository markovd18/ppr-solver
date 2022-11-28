#include <ocl/opencl.h>

#include <cstdlib>
#include <iostream>
#include <exception>
#include <vector>
#include <filesystem>

#include <ocl/ocl_statistics.h>
#include <util/string.h>
#include <data_loader.h>

namespace ocl {

    static std::filesystem::path load_kernel_path(const std::filesystem::path& path) {
        if (!std::filesystem::exists(path)) {
            throw std::invalid_argument("Kernel at path does not exist: " + path.string());
        }

        if (std::filesystem::is_directory(path)) {
            throw std::invalid_argument("Kernel is not a file: " + path.string());
        }

        return path;
    }

    static const auto current_path = std::filesystem::current_path();
    static const auto KERNEL_SOURCE_PATH = load_kernel_path(current_path / L"../src/ocl/sum_kernel.cl");

    class CDevice_Lookup_Result {
    public:
        CDevice_Lookup_Result() = default;
        explicit CDevice_Lookup_Result(std::string error_message) : m_error_message(std::move(error_message)) {}

        CDevice_Lookup_Result(std::string error_message, std::vector<cl::Device> devices)
            : m_error_message(std::move(error_message)), m_devices(std::move(devices)) {}

        bool Has_Error() const {
            return !m_error_message.empty();
        }

        const std::string Error_Message() const {
            return m_error_message;
        }

        void Set_Error_Message(std::string error_message) {
            m_error_message = error_message;
        }

        const std::vector<cl::Device>& Devices() const {
            return m_devices;
        }

        void addDevice(const cl::Device& device) {
            m_devices.push_back(device);
        }

    private:
        // Error when looking up device. If empty, no error occurred and device is populated.
        std::string m_error_message;
        std::vector<cl::Device> m_devices;
    };

    void lookup_ocl_devices(std::vector<std::wstring> device_names, CDevice_Lookup_Result& result) {
        cl::Device device;

        std::vector<cl::Platform> platforms;
        cl::Platform::get(&platforms);
        cl::Platform plat;

        const auto name_equals = [&device_names](const cl::Device& device) {
            const auto cl_device_name = ascii_string_to_wstring(device.getInfo<CL_DEVICE_NAME>());
            auto name_iter = device_names.begin();
            while (name_iter != device_names.end()) {
                if (*name_iter == cl_device_name) {
                    // TODO side-effect, je to vhodne reseni?
                    device_names.erase(name_iter);
                    return true;
                }

                name_iter++;
            }

            return false;
        };

        try {
            for (auto& platform : platforms) {
                std::vector<cl::Device> devices;
                platform.getDevices(CL_DEVICE_TYPE_GPU | CL_DEVICE_TYPE_CPU, &devices);

                auto device_iter = std::find_if(devices.begin(), devices.end(), name_equals);
                if (device_iter != devices.end()) {
                    result.addDevice(*device_iter);
                }

            }

            if (result.Devices().empty()) {
                result.Set_Error_Message("No OpenCL device found.");
            }
        } catch (const cl::Error& err) {
            result.Set_Error_Message(err.what());
        }
    }

    COCL_Stats_Calculator::COCL_Stats_Calculator(const std::vector<std::wstring>& device_names) {
        CDevice_Lookup_Result result;
        lookup_ocl_devices(device_names, result);
        if (result.Has_Error()) {
            throw std::invalid_argument(std::string("Error while looking up OCL device: ") + result.Error_Message());
            // TODO co tady? je vyjimka korektni reseni? sjednotit se stylem v CData_Loader
        }
        
        m_devices = result.Devices();
    }

    CStatistics COCL_Stats_Calculator::Analyze_Vector(const std::vector<double>& data) {
        const std::size_t batch_size = data.size() / m_devices.size();
        
        const std::string kernel_code = load_text_file(KERNEL_SOURCE_PATH);
        //const std::string reduce_kernel_code = load_text_file(REDUCE_KERNEL_PATH);

        if (kernel_code.empty()) {
            throw std::runtime_error("Error while loading kernels");
        }

        cl::Program::Sources sources{ kernel_code };
        const auto& device = m_devices.front(); // TODO rozdelit na N casti a dat kazdymu zarizeni

        cl::Context device_context{ device };

        // program has to be compiled for every device
        cl::Program program(device_context, sources);

        try {
             program.build(device, "-cl-std=CL2.0");
        } catch (...) {
            cl_int buildErr = CL_SUCCESS;
            auto buildInfo = program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(&buildErr);
            std::string errors;
            for (auto& pair : buildInfo) {
                errors.append(pair.second).push_back('\n');
            }
            // TODO sjednotit se stylem CData_Loader
            throw std::runtime_error("Error while building OpenCL program: " + errors);
        }

        cl::Kernel sum_kernel(program, "sum_kernel");
        cl::Kernel reduce_kernel(program, "reduce_kernel");
        
        static const std::size_t count = 256;
        const std::size_t group_count = std::ceil(data.size() / double(count));
        
        std::vector<double> result_n(data.size());
        std::vector<double> result_M1(data.size());
        std::vector<double> result_M2(data.size());
        std::vector<double> result_M3(data.size());
        std::vector<double> result_M4(data.size());

        //buffery, se kterymi bude pracovat kernel
        cl::Buffer numbers_buffer{ 
            device_context, 
            CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, 
            sizeof(data), const_cast<double*>(data.data())
        };

        
        // TODO lok·lnÌ pamÏù na OCL se nechov· podle oËek·v·nÌ, nespr·vnÈ v˝sledky
        cl::Buffer n_buffer{ device_context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, sizeof(result_n), result_n.data() };
        cl::Buffer M1_buffer{ device_context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, sizeof(result_M1), result_M1.data() };
        cl::Buffer M2_buffer{ device_context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, sizeof(result_M2), result_M2.data() };
        cl::Buffer M3_buffer{ device_context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, sizeof(result_M3), result_M3.data() };
        cl::Buffer M4_buffer{ device_context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, sizeof(result_M4), result_M4.data() };

        try {
            sum_kernel.setArg(0, numbers_buffer);
            sum_kernel.setArg(1, n_buffer);
            sum_kernel.setArg(2, M1_buffer);
            sum_kernel.setArg(3, M2_buffer);
            sum_kernel.setArg(4, M3_buffer);
            sum_kernel.setArg(5, M4_buffer);

            cl::CommandQueue queue(device_context, device, 0);

            queue.enqueueNDRangeKernel(sum_kernel, cl::NullRange, cl::NDRange(data.size()), cl::NDRange(count));
            queue.finish();

        } catch (const cl::Error& error) {
            const std::string error_message = std::to_string(error.err()) + ": " + error.what();
            throw std::runtime_error(std::move(error_message.c_str()));
        }
       
        // TODO
        return CStatistics(/*result_n, result_M1, result_M2, result_M3, result_M4*/);
    }

}
