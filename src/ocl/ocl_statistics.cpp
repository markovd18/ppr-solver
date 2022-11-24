#include <ocl/opencl.h>

#include <cstdlib>
#include <iostream>
#include <exception>
#include <vector>

#include <ocl/ocl_statistics.h>
#include <util/string.h>

namespace ocl {
    
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
            const auto name_iter = device_names.begin();
            while (name_iter != device_names.end()) {
                if (*name_iter == cl_device_name) {
                    // TODO side-effect, je to vhodne reseni?
                    device_names.erase(name_iter);
                    return true;
                }
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
        } catch (const cl::Error& err) {
            result.Set_Error_Message(err.what());
        }
    }

    COCL_Stats_Calculator::COCL_Stats_Calculator(const std::vector<std::wstring>& device_names) {
        CDevice_Lookup_Result result;
        lookup_ocl_devices(device_names, result);
        if (result.Has_Error()) {
            throw std::invalid_argument(std::string("Error while looking up OCL device: ") + result.Error_Message());
            // TODO co tady? je vyjimka korektni reseni?
        }

        m_devices = result.Devices();
    }

    CStatistics COCL_Stats_Calculator::Analyze_Vector(const std::vector<double>& data) {
        return CStatistics();
    }

}
