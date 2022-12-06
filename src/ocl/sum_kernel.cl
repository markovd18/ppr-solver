#define LOCAL_GROUP_XDIM 256

void reduce_work_group(double* local_n, 
                        double* local_M1, 
                        double* local_M2, 
                        double* local_M3, 
                        double* local_M4, 
                        double* result_n, 
                        double* result_M1, 
                        double* result_M2, 
                        double* result_M3, 
                        double* result_M4) {
    uint local_id = get_local_id(0);
    uint id = get_global_id(0);
    uint work_group_id = get_group_id(0);
    
    uint dist = LOCAL_GROUP_XDIM;   // get_local_size(0)
    while (dist > 1) {
        dist >>= 1;
        if (local_id < dist) {
            uint other_id = local_id + dist;
            
            double prev_n = local_n[local_id];
            double new_n = prev_n + local_n[other_id];
            double prev_M2 = local_M2[local_id];
            double prev_M3 = local_M3[local_id];

            double delta = local_M1[local_id] - local_M1[other_id];
            double delta2 = delta * delta;
            double delta3 = delta * delta2;
            double delta4 = delta2 * delta2;
            
            local_M1[local_id] = (local_n[other_id] * local_M1[other_id] + prev_n * local_M1[local_id]) / new_n;
            local_M2[local_id] = local_M2[other_id] + local_M2[local_id] + delta2 * local_n[other_id] * prev_n / new_n;
            
            local_M3[local_id] = local_M3[other_id] + local_M3[local_id] + 
                delta3 * local_n[other_id] * prev_n * (local_n[other_id] - prev_n) / (new_n * new_n);

            local_M3[local_id] += 3.0 * delta * (local_n[other_id] * prev_M2 - prev_n * local_M2[other_id]) / new_n;

            local_M4[local_id] = local_M4[other_id] + local_M4[local_id] + delta4 * local_n[other_id] * prev_n * 
                (local_n[other_id] * local_n[other_id] - local_n[other_id] * prev_n + prev_n * prev_n) /
                (new_n * new_n * new_n);

            local_M4[local_id] += 6.0 * delta2 * (local_n[other_id] * local_n[other_id] * prev_M2 + 
                prev_n * prev_n * local_M2[other_id]) / (new_n * new_n) + 
                4.0 * delta * (local_n[other_id] * prev_M3 - prev_n * local_M3[other_id]) / new_n;
            
            local_n[local_id] = new_n; 
        }

        barrier(CLK_LOCAL_MEM_FENCE);
    }

    if (local_id == 0) {
        result_n[work_group_id] = local_n[local_id];
        result_M1[work_group_id] = local_M1[local_id];
        result_M2[work_group_id] = local_M2[local_id];
        result_M3[work_group_id] = local_M3[local_id];
        result_M4[work_group_id] = local_M4[local_id];
        //printf("group %d - n: %f, M1: %f, M2: %f, M3: %f, M4: %lf\n", work_group_id, result_n[work_group_id], result_M1[work_group_id], result_M2[work_group_id], result_M3[work_group_id], result_M4[work_group_id]);
    }                
}

__kernel __attribute__((reqd_work_group_size(LOCAL_GROUP_XDIM, 1, 1))) 
void reduce_kernel(__global const double* numbers,
                    const double number_count,
                    __global double* g_n,
                    __global double* g_M1,
                    __global double* g_M2,
                    __global double* g_M3,
                    __global double* g_M4) {
    uint local_id = get_local_id(0);
    uint id = get_global_id(0);
    uint work_group_id = get_group_id(0);

    __local double n[LOCAL_GROUP_XDIM]; 
    __local double M1[LOCAL_GROUP_XDIM]; 
    __local double M2[LOCAL_GROUP_XDIM];
    __local double M3[LOCAL_GROUP_XDIM];
    __local double M4[LOCAL_GROUP_XDIM];

    n[local_id] = 0;
    M1[local_id] = 0;
    M2[local_id] = 0;
    M3[local_id] = 0;
    M4[local_id] = 0;

    n[local_id] = 1;
    M1[local_id] = numbers[id];

    uint other_id = id + LOCAL_GROUP_XDIM;
    while (other_id < number_count) {
        uint n1 = n[local_id];
        uint n2 = n1 + 1;

        double delta = numbers[other_id] - M1[local_id];
        double delta_n = delta / n2;
        double delta_n2 = delta_n * delta_n;
        double term1 = delta * delta_n * n1;

        M1[local_id] += delta_n;

        M4[local_id] += term1 * delta_n2 * (n2 * n2 - 3 * n2 + 3) + 6 * delta_n2 * M2[local_id] - 4 * delta_n * M3[local_id];
        M3[local_id] += term1 * delta_n * (n2 - 2) - 3 * delta_n * M2[local_id];
        M2[local_id] += term1;
        n[local_id] = n2;

        other_id += LOCAL_GROUP_XDIM;
    }

    barrier(CLK_LOCAL_MEM_FENCE);

    reduce_work_group(n, M1, M2, M3, M4, g_n, g_M1, g_M2, g_M3, g_M4);
}

