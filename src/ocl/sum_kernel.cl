#define LOCAL_GROUP_XDIM 256

// TODO zamyslet se nad timto resenim a pripadne cely prepsat

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
            uint other_id = id + dist;
            
            double prev_n = local_n[local_id];
            double new_n = prev_n + local_n[other_id];
            // if (id == 0) {
             //   printf("local ID: %d, other ID: %d, old n: %d, other n: %d, new n: %d", id, other_id, prev_n, local_n[other_id], new_n);
            //}
            double delta = local_M1[local_id] - local_M1[other_id];
            double delta2 = delta * delta;
            double delta3 = delta * delta2;
            double delta4 = delta2 * delta2;

            local_M1[local_id] = (local_n[other_id] * local_M1[other_id] + prev_n * local_M1[local_id]) / new_n;

            local_M2[local_id] = local_M2[other_id] + local_M2[local_id] + delta2 * local_n[other_id] * prev_n / new_n;

            local_M3[local_id] = local_M3[other_id] + local_M3[local_id] + 
                delta3 * local_n[other_id] * prev_n * (local_n[other_id] * prev_n) / (new_n * new_n);

            local_M3[local_id] += 3.0 * delta * (local_n[other_id] * local_M2[local_id] - prev_n * local_M2[other_id]) / new_n;

            local_M4[local_id] = local_M4[other_id] + local_M4[local_id] + delta4 * local_n[other_id] * prev_n * 
                (local_n[other_id] * local_n[other_id] - local_n[other_id] * prev_n + prev_n * prev_n) /
                (new_n * new_n * new_n);

            local_M4[local_id] += 6.0 * delta2 * (local_n[other_id] * local_n[other_id] * local_M2[local_id] + 
                prev_n * prev_n * local_M2[other_id]) / (new_n * new_n) + 
                4.0 * delta * (local_n[other_id] * local_M3[local_id] - prev_n * local_M3[other_id]) / new_n;
            
            local_n[local_id] = new_n; 
        }

        // barrier(CLK_GLOBAL_MEM_FENCE);
    }

    if (local_id == 0) {
        // printf("n: %d, M1: %d, M2: %d, M3: %d, M4: %d\n", local_n[local_id], local_M1[local_id], local_M2[local_id], local_M3[local_id], local_M4[local_id]);
        result_n[work_group_id] = local_n[local_id];
        result_M1[work_group_id] = local_M1[local_id];
        result_M2[work_group_id] = local_M2[local_id];
        result_M3[work_group_id] = local_M3[local_id];
        result_M4[work_group_id] = local_M4[local_id];
    }                
}

__kernel __attribute__((reqd_work_group_size(LOCAL_GROUP_XDIM, 1, 1))) 
void sum_kernel(__global const double* numbers, 
                    __global double* g_n,
                    __global double* g_M1,
                    __global double* g_M2,
                    __global double* g_M3,
                    __global double* g_M4) {
    uint id = get_global_id(0);
    uint local_id = get_local_id(0);
    
    __local double n[LOCAL_GROUP_XDIM]; 
    __local double M1[LOCAL_GROUP_XDIM]; 
    __local double M2[LOCAL_GROUP_XDIM];
    __local double M3[LOCAL_GROUP_XDIM];
    __local double M4[LOCAL_GROUP_XDIM];

    // uint n1 = n[local_id];
    // uint n2 = n1 + 1;   // 1
     n[local_id] = 1;
    // g_n[id] = 1;
    // double delta = numbers[id] - M1[local_id];  // numbers[id]
    // double delta_n = delta / n2;    // numbers[id]
    // double delta_n2 = delta_n * delta_n;    // numbers[id] * numbers[id] but not needed
    // double term1 = delta * delta_n * n1; // 0

    // g_M1[id] = numbers[id];
    // M1[local_id] += delta_n;    // numbers[id]
    M1[local_id] = numbers[id];

    // only first sum - these will always be 0 so no need to compute that at all
    // M4[local_id] += term1 * delta_n2 * (n2 * n2 - 3 * n2 + 3) + 6 * delta_n2 * M2[local_id] - 4 * delta_n * M3[local_id];
    // M3[local_id] += term1 * delta_n * (n2 - 2) - 3 * delta_n * M2[local_id];
    // M2[local_id] += term1;

    barrier(CLK_LOCAL_MEM_FENCE);
    // if (local_id == 0) {
        // printf("local N: %d\n", g_n[local_id]);
    // }
    reduce_work_group(n, M1, M2, M3, M4, g_n, g_M1, g_M2, g_M3, g_M4);
}

__kernel __attribute__((reqd_work_group_size(LOCAL_GROUP_XDIM, 1, 1))) 
void reduce_kernel(__global const double* n,
                    __global const double* M1,
                    __global const double* M2,
                    __global const double* M3,
                    __global const double* M4,
                    __global double* result_n,
                    __global double* result_M1,
                    __global double* result_M2,
                    __global double* result_M3,
                    __global double* result_M4) {
    uint local_id = get_local_id(0);

    __local double local_n[LOCAL_GROUP_XDIM];
    __local double local_M1[LOCAL_GROUP_XDIM];
    __local double local_M2[LOCAL_GROUP_XDIM];
    __local double local_M3[LOCAL_GROUP_XDIM];
    __local double local_M4[LOCAL_GROUP_XDIM];

    local_n[local_id] = n[local_id];
    local_M1[local_id] = M1[local_id];
    local_M2[local_id] = M2[local_id];
    local_M3[local_id] = M3[local_id];
    local_M4[local_id] = M4[local_id];

    barrier(CLK_LOCAL_MEM_FENCE);

    reduce_work_group(local_n, local_M1, local_M2, local_M3, local_M4, result_n, result_M1, result_M2, result_M3, result_M4);
    // end of kernel
}
