#pragma once
#include "math.hpp"



class EKF {
    // math
    math::quarternion m_orientation; // 4
    math::vec3& m_gyro_biases; // 3
    double m_gyro_covariances_buf_A[49]; // (4 + 3) ** 2
    double m_gyro_covariances_buf_B[49]; // (4 + 3) ** 2
    double * m_gyro_covariances = m_gyro_covariances_buf_A; // Old covar is stored here.
    double * m_gyro_covariances_new = m_gyro_covariances_buf_B; // New covar goes here
    // math::Vec
    double m_gyroscope_variance[9];





    inline void initialize_covariances() {
        for(int i = 0; i < 3; i ++) {
            for(int j = 0; j < 3; j++) {
                m_gyroscope_variance[i*3+j] = 0;
            }
        }
        m_gyroscope_variance[0] = m_gyroscope_variance[4] = m_gyroscope_variance[8] = 0.1f;
        for(int i = 0; i < 7; i ++) {
            for(int j = 0; j < 7; j++) {
                m_gyro_covariances_buf_A[i*7+j] = m_gyro_covariances_buf_B[i*7+j] = (i == j) * 10000;
            }
        }
    }

    inline void predict(const math::vec3& p_angular_rates) {
        math::quarternion orientation_new;
        generated_predict(m_orientation, p_angular_rates, m_gyro_biases, orientation_new);

        generated_predict_update_covariances(m_orientation, p_angular_rates, m_gyro_biases, m_gyroscope_variance, m_gyro_covariances, m_gyro_covariances_new);
        // m_gyro_covariances

        std::swap(m_gyro_covariances, m_gyro_covariances_new); // Swap Covariance buffers. The new one is now the current one.
        m_orientation = orientation_new; // Update Orientation
    }

    inline void update_accelerometer_gravity() {

    }

    inline void generated_predict(const math::quarternion& q, const math::vec3& w, const math::vec3& b, math::quarternion& q_new /*, const math::vec3& b_new */) {

        q_new.w = q.w + q.x*(b.x - w.x) + q.y*(b.y - w.y) + q.z*(b.z - w.z);
        q_new.x = q.w*(-b.x + w.x) + q.x + q.y*(-b.z + w.z) + q.z*(b.y - w.y);
        q_new.y = q.w*(-b.y + w.y) + q.x*(b.z - w.z) + q.y + q.z*(-b.x + w.x);
        q_new.z = q.w*(-b.z + w.z) + q.x*(-b.y + w.y) + q.y*(b.x - w.x) + q.z;
        // b_new.x = b.x;
        // b_new.y = b.y;
        // b_new.z = b.z;
    }

    inline void generated_predict_update_covariances(const math::quarternion& q, const math::vec3& w, const math::vec3& b, double * W, double * P, double * p_pred) {
        // Generated using sympy.
        // ../derive/rotation_kalman.ipynb
        p_pred[0] = -q.x*(-q.x*W[0] - q.y*W[3] - q.z*W[6]) + q.x*(q.x*P[32] + q.y*P[39] + q.z*P[46] + (b.x - w.x)*P[11] + (b.y - w.y)*P[18] + (b.z - w.z)*P[25] + P[4]) + q.x*P[28] - q.y*(-q.x*W[1] - q.y*W[4] - q.z*W[7]) + q.y*(q.x*P[33] + q.y*P[40] + q.z*P[47] + (b.x - w.x)*P[12] + (b.y - w.y)*P[19] + (b.z - w.z)*P[26] + P[5]) + q.y*P[35] - q.z*(-q.x*W[2] - q.y*W[5] - q.z*W[8]) + q.z*(q.x*P[34] + q.y*P[41] + q.z*P[48] + (b.x - w.x)*P[13] + (b.y - w.y)*P[20] + (b.z - w.z)*P[27] + P[6]) + q.z*P[42] + (b.x - w.x)*(q.x*P[29] + q.y*P[36] + q.z*P[43] + (b.x - w.x)*P[8] + (b.y - w.y)*P[15] + (b.z - w.z)*P[22] + P[1]) + (b.x - w.x)*P[7] + (b.y - w.y)*(q.x*P[30] + q.y*P[37] + q.z*P[44] + (b.x - w.x)*P[9] + (b.y - w.y)*P[16] + (b.z - w.z)*P[23] + P[2]) + (b.y - w.y)*P[14] + (b.z - w.z)*(q.x*P[31] + q.y*P[38] + q.z*P[45] + (b.x - w.x)*P[10] + (b.y - w.y)*P[17] + (b.z - w.z)*P[24] + P[3]) + (b.z - w.z)*P[21] + P[0];
        p_pred[1] = q.w*(-q.x*W[0] - q.y*W[3] - q.z*W[6]) - q.w*(q.x*P[32] + q.y*P[39] + q.z*P[46] + (b.x - w.x)*P[11] + (b.y - w.y)*P[18] + (b.z - w.z)*P[25] + P[4]) + q.x*P[29] + q.y*(-q.x*W[2] - q.y*W[5] - q.z*W[8]) - q.y*(q.x*P[34] + q.y*P[41] + q.z*P[48] + (b.x - w.x)*P[13] + (b.y - w.y)*P[20] + (b.z - w.z)*P[27] + P[6]) + q.y*P[36] - q.z*(-q.x*W[1] - q.y*W[4] - q.z*W[7]) + q.z*(q.x*P[33] + q.y*P[40] + q.z*P[47] + (b.x - w.x)*P[12] + (b.y - w.y)*P[19] + (b.z - w.z)*P[26] + P[5]) + q.z*P[43] + (-b.x + w.x)*(q.x*P[28] + q.y*P[35] + q.z*P[42] + (b.x - w.x)*P[7] + (b.y - w.y)*P[14] + (b.z - w.z)*P[21] + P[0]) + (b.x - w.x)*P[8] + (b.y - w.y)*(q.x*P[31] + q.y*P[38] + q.z*P[45] + (b.x - w.x)*P[10] + (b.y - w.y)*P[17] + (b.z - w.z)*P[24] + P[3]) + (b.y - w.y)*P[15] + (-b.z + w.z)*(q.x*P[30] + q.y*P[37] + q.z*P[44] + (b.x - w.x)*P[9] + (b.y - w.y)*P[16] + (b.z - w.z)*P[23] + P[2]) + (b.z - w.z)*P[22] + P[1];
        p_pred[2] = q.w*(-q.x*W[1] - q.y*W[4] - q.z*W[7]) - q.w*(q.x*P[33] + q.y*P[40] + q.z*P[47] + (b.x - w.x)*P[12] + (b.y - w.y)*P[19] + (b.z - w.z)*P[26] + P[5]) - q.x*(-q.x*W[2] - q.y*W[5] - q.z*W[8]) + q.x*(q.x*P[34] + q.y*P[41] + q.z*P[48] + (b.x - w.x)*P[13] + (b.y - w.y)*P[20] + (b.z - w.z)*P[27] + P[6]) + q.x*P[30] + q.y*P[37] + q.z*(-q.x*W[0] - q.y*W[3] - q.z*W[6]) - q.z*(q.x*P[32] + q.y*P[39] + q.z*P[46] + (b.x - w.x)*P[11] + (b.y - w.y)*P[18] + (b.z - w.z)*P[25] + P[4]) + q.z*P[44] + (-b.x + w.x)*(q.x*P[31] + q.y*P[38] + q.z*P[45] + (b.x - w.x)*P[10] + (b.y - w.y)*P[17] + (b.z - w.z)*P[24] + P[3]) + (b.x - w.x)*P[9] + (-b.y + w.y)*(q.x*P[28] + q.y*P[35] + q.z*P[42] + (b.x - w.x)*P[7] + (b.y - w.y)*P[14] + (b.z - w.z)*P[21] + P[0]) + (b.y - w.y)*P[16] + (b.z - w.z)*(q.x*P[29] + q.y*P[36] + q.z*P[43] + (b.x - w.x)*P[8] + (b.y - w.y)*P[15] + (b.z - w.z)*P[22] + P[1]) + (b.z - w.z)*P[23] + P[2];
        p_pred[3] = q.w*(-q.x*W[2] - q.y*W[5] - q.z*W[8]) - q.w*(q.x*P[34] + q.y*P[41] + q.z*P[48] + (b.x - w.x)*P[13] + (b.y - w.y)*P[20] + (b.z - w.z)*P[27] + P[6]) + q.x*(-q.x*W[1] - q.y*W[4] - q.z*W[7]) - q.x*(q.x*P[33] + q.y*P[40] + q.z*P[47] + (b.x - w.x)*P[12] + (b.y - w.y)*P[19] + (b.z - w.z)*P[26] + P[5]) + q.x*P[31] - q.y*(-q.x*W[0] - q.y*W[3] - q.z*W[6]) + q.y*(q.x*P[32] + q.y*P[39] + q.z*P[46] + (b.x - w.x)*P[11] + (b.y - w.y)*P[18] + (b.z - w.z)*P[25] + P[4]) + q.y*P[38] + q.z*P[45] + (b.x - w.x)*(q.x*P[30] + q.y*P[37] + q.z*P[44] + (b.x - w.x)*P[9] + (b.y - w.y)*P[16] + (b.z - w.z)*P[23] + P[2]) + (b.x - w.x)*P[10] + (-b.y + w.y)*(q.x*P[29] + q.y*P[36] + q.z*P[43] + (b.x - w.x)*P[8] + (b.y - w.y)*P[15] + (b.z - w.z)*P[22] + P[1]) + (b.y - w.y)*P[17] + (-b.z + w.z)*(q.x*P[28] + q.y*P[35] + q.z*P[42] + (b.x - w.x)*P[7] + (b.y - w.y)*P[14] + (b.z - w.z)*P[21] + P[0]) + (b.z - w.z)*P[24] + P[3];
        p_pred[4] = q.x*P[32] + q.y*P[39] + q.z*P[46] + (b.x - w.x)*P[11] + (b.y - w.y)*P[18] + (b.z - w.z)*P[25] + P[4];
        p_pred[5] = q.x*P[33] + q.y*P[40] + q.z*P[47] + (b.x - w.x)*P[12] + (b.y - w.y)*P[19] + (b.z - w.z)*P[26] + P[5];
        p_pred[6] = q.x*P[34] + q.y*P[41] + q.z*P[48] + (b.x - w.x)*P[13] + (b.y - w.y)*P[20] + (b.z - w.z)*P[27] + P[6];
        p_pred[7] = -q.w*P[28] - q.x*(q.w*W[0] + q.y*W[6] - q.z*W[3]) + q.x*(-q.w*P[32] - q.y*P[46] + q.z*P[39] + (-b.x + w.x)*P[4] + (b.y - w.y)*P[25] + (-b.z + w.z)*P[18] + P[11]) - q.y*(q.w*W[1] + q.y*W[7] - q.z*W[4]) + q.y*(-q.w*P[33] - q.y*P[47] + q.z*P[40] + (-b.x + w.x)*P[5] + (b.y - w.y)*P[26] + (-b.z + w.z)*P[19] + P[12]) - q.y*P[42] - q.z*(q.w*W[2] + q.y*W[8] - q.z*W[5]) + q.z*(-q.w*P[34] - q.y*P[48] + q.z*P[41] + (-b.x + w.x)*P[6] + (b.y - w.y)*P[27] + (-b.z + w.z)*P[20] + P[13]) + q.z*P[35] + (-b.x + w.x)*P[0] + (b.x - w.x)*(-q.w*P[29] - q.y*P[43] + q.z*P[36] + (-b.x + w.x)*P[1] + (b.y - w.y)*P[22] + (-b.z + w.z)*P[15] + P[8]) + (b.y - w.y)*(-q.w*P[30] - q.y*P[44] + q.z*P[37] + (-b.x + w.x)*P[2] + (b.y - w.y)*P[23] + (-b.z + w.z)*P[16] + P[9]) + (b.y - w.y)*P[21] + (-b.z + w.z)*P[14] + (b.z - w.z)*(-q.w*P[31] - q.y*P[45] + q.z*P[38] + (-b.x + w.x)*P[3] + (b.y - w.y)*P[24] + (-b.z + w.z)*P[17] + P[10]) + P[7];
        p_pred[8] = q.w*(q.w*W[0] + q.y*W[6] - q.z*W[3]) - q.w*(-q.w*P[32] - q.y*P[46] + q.z*P[39] + (-b.x + w.x)*P[4] + (b.y - w.y)*P[25] + (-b.z + w.z)*P[18] + P[11]) - q.w*P[29] + q.y*(q.w*W[2] + q.y*W[8] - q.z*W[5]) - q.y*(-q.w*P[34] - q.y*P[48] + q.z*P[41] + (-b.x + w.x)*P[6] + (b.y - w.y)*P[27] + (-b.z + w.z)*P[20] + P[13]) - q.y*P[43] - q.z*(q.w*W[1] + q.y*W[7] - q.z*W[4]) + q.z*(-q.w*P[33] - q.y*P[47] + q.z*P[40] + (-b.x + w.x)*P[5] + (b.y - w.y)*P[26] + (-b.z + w.z)*P[19] + P[12]) + q.z*P[36] + (-b.x + w.x)*(-q.w*P[28] - q.y*P[42] + q.z*P[35] + (-b.x + w.x)*P[0] + (b.y - w.y)*P[21] + (-b.z + w.z)*P[14] + P[7]) + (-b.x + w.x)*P[1] + (b.y - w.y)*(-q.w*P[31] - q.y*P[45] + q.z*P[38] + (-b.x + w.x)*P[3] + (b.y - w.y)*P[24] + (-b.z + w.z)*P[17] + P[10]) + (b.y - w.y)*P[22] + (-b.z + w.z)*(-q.w*P[30] - q.y*P[44] + q.z*P[37] + (-b.x + w.x)*P[2] + (b.y - w.y)*P[23] + (-b.z + w.z)*P[16] + P[9]) + (-b.z + w.z)*P[15] + P[8];
        p_pred[9] = q.w*(q.w*W[1] + q.y*W[7] - q.z*W[4]) - q.w*(-q.w*P[33] - q.y*P[47] + q.z*P[40] + (-b.x + w.x)*P[5] + (b.y - w.y)*P[26] + (-b.z + w.z)*P[19] + P[12]) - q.w*P[30] - q.x*(q.w*W[2] + q.y*W[8] - q.z*W[5]) + q.x*(-q.w*P[34] - q.y*P[48] + q.z*P[41] + (-b.x + w.x)*P[6] + (b.y - w.y)*P[27] + (-b.z + w.z)*P[20] + P[13]) - q.y*P[44] + q.z*(q.w*W[0] + q.y*W[6] - q.z*W[3]) - q.z*(-q.w*P[32] - q.y*P[46] + q.z*P[39] + (-b.x + w.x)*P[4] + (b.y - w.y)*P[25] + (-b.z + w.z)*P[18] + P[11]) + q.z*P[37] + (-b.x + w.x)*(-q.w*P[31] - q.y*P[45] + q.z*P[38] + (-b.x + w.x)*P[3] + (b.y - w.y)*P[24] + (-b.z + w.z)*P[17] + P[10]) + (-b.x + w.x)*P[2] + (-b.y + w.y)*(-q.w*P[28] - q.y*P[42] + q.z*P[35] + (-b.x + w.x)*P[0] + (b.y - w.y)*P[21] + (-b.z + w.z)*P[14] + P[7]) + (b.y - w.y)*P[23] + (-b.z + w.z)*P[16] + (b.z - w.z)*(-q.w*P[29] - q.y*P[43] + q.z*P[36] + (-b.x + w.x)*P[1] + (b.y - w.y)*P[22] + (-b.z + w.z)*P[15] + P[8]) + P[9];
        p_pred[10] = q.w*(q.w*W[2] + q.y*W[8] - q.z*W[5]) - q.w*(-q.w*P[34] - q.y*P[48] + q.z*P[41] + (-b.x + w.x)*P[6] + (b.y - w.y)*P[27] + (-b.z + w.z)*P[20] + P[13]) - q.w*P[31] + q.x*(q.w*W[1] + q.y*W[7] - q.z*W[4]) - q.x*(-q.w*P[33] - q.y*P[47] + q.z*P[40] + (-b.x + w.x)*P[5] + (b.y - w.y)*P[26] + (-b.z + w.z)*P[19] + P[12]) - q.y*(q.w*W[0] + q.y*W[6] - q.z*W[3]) + q.y*(-q.w*P[32] - q.y*P[46] + q.z*P[39] + (-b.x + w.x)*P[4] + (b.y - w.y)*P[25] + (-b.z + w.z)*P[18] + P[11]) - q.y*P[45] + q.z*P[38] + (-b.x + w.x)*P[3] + (b.x - w.x)*(-q.w*P[30] - q.y*P[44] + q.z*P[37] + (-b.x + w.x)*P[2] + (b.y - w.y)*P[23] + (-b.z + w.z)*P[16] + P[9]) + (-b.y + w.y)*(-q.w*P[29] - q.y*P[43] + q.z*P[36] + (-b.x + w.x)*P[1] + (b.y - w.y)*P[22] + (-b.z + w.z)*P[15] + P[8]) + (b.y - w.y)*P[24] + (-b.z + w.z)*(-q.w*P[28] - q.y*P[42] + q.z*P[35] + (-b.x + w.x)*P[0] + (b.y - w.y)*P[21] + (-b.z + w.z)*P[14] + P[7]) + (-b.z + w.z)*P[17] + P[10];
        p_pred[11] = -q.w*P[32] - q.y*P[46] + q.z*P[39] + (-b.x + w.x)*P[4] + (b.y - w.y)*P[25] + (-b.z + w.z)*P[18] + P[11];
        p_pred[12] = -q.w*P[33] - q.y*P[47] + q.z*P[40] + (-b.x + w.x)*P[5] + (b.y - w.y)*P[26] + (-b.z + w.z)*P[19] + P[12];
        p_pred[13] = -q.w*P[34] - q.y*P[48] + q.z*P[41] + (-b.x + w.x)*P[6] + (b.y - w.y)*P[27] + (-b.z + w.z)*P[20] + P[13];
        p_pred[14] = -q.w*P[35] - q.x*(q.w*W[3] - q.x*W[6] + q.z*W[0]) + q.x*(-q.w*P[39] + q.x*P[46] - q.z*P[32] + (-b.x + w.x)*P[25] + (-b.y + w.y)*P[4] + (b.z - w.z)*P[11] + P[18]) + q.x*P[42] - q.y*(q.w*W[4] - q.x*W[7] + q.z*W[1]) + q.y*(-q.w*P[40] + q.x*P[47] - q.z*P[33] + (-b.x + w.x)*P[26] + (-b.y + w.y)*P[5] + (b.z - w.z)*P[12] + P[19]) - q.z*(q.w*W[5] - q.x*W[8] + q.z*W[2]) + q.z*(-q.w*P[41] + q.x*P[48] - q.z*P[34] + (-b.x + w.x)*P[27] + (-b.y + w.y)*P[6] + (b.z - w.z)*P[13] + P[20]) - q.z*P[28] + (-b.x + w.x)*P[21] + (b.x - w.x)*(-q.w*P[36] + q.x*P[43] - q.z*P[29] + (-b.x + w.x)*P[22] + (-b.y + w.y)*P[1] + (b.z - w.z)*P[8] + P[15]) + (-b.y + w.y)*P[0] + (b.y - w.y)*(-q.w*P[37] + q.x*P[44] - q.z*P[30] + (-b.x + w.x)*P[23] + (-b.y + w.y)*P[2] + (b.z - w.z)*P[9] + P[16]) + (b.z - w.z)*(-q.w*P[38] + q.x*P[45] - q.z*P[31] + (-b.x + w.x)*P[24] + (-b.y + w.y)*P[3] + (b.z - w.z)*P[10] + P[17]) + (b.z - w.z)*P[7] + P[14];
        p_pred[15] = q.w*(q.w*W[3] - q.x*W[6] + q.z*W[0]) - q.w*(-q.w*P[39] + q.x*P[46] - q.z*P[32] + (-b.x + w.x)*P[25] + (-b.y + w.y)*P[4] + (b.z - w.z)*P[11] + P[18]) - q.w*P[36] + q.x*P[43] + q.y*(q.w*W[5] - q.x*W[8] + q.z*W[2]) - q.y*(-q.w*P[41] + q.x*P[48] - q.z*P[34] + (-b.x + w.x)*P[27] + (-b.y + w.y)*P[6] + (b.z - w.z)*P[13] + P[20]) - q.z*(q.w*W[4] - q.x*W[7] + q.z*W[1]) + q.z*(-q.w*P[40] + q.x*P[47] - q.z*P[33] + (-b.x + w.x)*P[26] + (-b.y + w.y)*P[5] + (b.z - w.z)*P[12] + P[19]) - q.z*P[29] + (-b.x + w.x)*(-q.w*P[35] + q.x*P[42] - q.z*P[28] + (-b.x + w.x)*P[21] + (-b.y + w.y)*P[0] + (b.z - w.z)*P[7] + P[14]) + (-b.x + w.x)*P[22] + (-b.y + w.y)*P[1] + (b.y - w.y)*(-q.w*P[38] + q.x*P[45] - q.z*P[31] + (-b.x + w.x)*P[24] + (-b.y + w.y)*P[3] + (b.z - w.z)*P[10] + P[17]) + (-b.z + w.z)*(-q.w*P[37] + q.x*P[44] - q.z*P[30] + (-b.x + w.x)*P[23] + (-b.y + w.y)*P[2] + (b.z - w.z)*P[9] + P[16]) + (b.z - w.z)*P[8] + P[15];
        p_pred[16] = q.w*(q.w*W[4] - q.x*W[7] + q.z*W[1]) - q.w*(-q.w*P[40] + q.x*P[47] - q.z*P[33] + (-b.x + w.x)*P[26] + (-b.y + w.y)*P[5] + (b.z - w.z)*P[12] + P[19]) - q.w*P[37] - q.x*(q.w*W[5] - q.x*W[8] + q.z*W[2]) + q.x*(-q.w*P[41] + q.x*P[48] - q.z*P[34] + (-b.x + w.x)*P[27] + (-b.y + w.y)*P[6] + (b.z - w.z)*P[13] + P[20]) + q.x*P[44] + q.z*(q.w*W[3] - q.x*W[6] + q.z*W[0]) - q.z*(-q.w*P[39] + q.x*P[46] - q.z*P[32] + (-b.x + w.x)*P[25] + (-b.y + w.y)*P[4] + (b.z - w.z)*P[11] + P[18]) - q.z*P[30] + (-b.x + w.x)*(-q.w*P[38] + q.x*P[45] - q.z*P[31] + (-b.x + w.x)*P[24] + (-b.y + w.y)*P[3] + (b.z - w.z)*P[10] + P[17]) + (-b.x + w.x)*P[23] + (-b.y + w.y)*(-q.w*P[35] + q.x*P[42] - q.z*P[28] + (-b.x + w.x)*P[21] + (-b.y + w.y)*P[0] + (b.z - w.z)*P[7] + P[14]) + (-b.y + w.y)*P[2] + (b.z - w.z)*(-q.w*P[36] + q.x*P[43] - q.z*P[29] + (-b.x + w.x)*P[22] + (-b.y + w.y)*P[1] + (b.z - w.z)*P[8] + P[15]) + (b.z - w.z)*P[9] + P[16];
        p_pred[17] = q.w*(q.w*W[5] - q.x*W[8] + q.z*W[2]) - q.w*(-q.w*P[41] + q.x*P[48] - q.z*P[34] + (-b.x + w.x)*P[27] + (-b.y + w.y)*P[6] + (b.z - w.z)*P[13] + P[20]) - q.w*P[38] + q.x*(q.w*W[4] - q.x*W[7] + q.z*W[1]) - q.x*(-q.w*P[40] + q.x*P[47] - q.z*P[33] + (-b.x + w.x)*P[26] + (-b.y + w.y)*P[5] + (b.z - w.z)*P[12] + P[19]) + q.x*P[45] - q.y*(q.w*W[3] - q.x*W[6] + q.z*W[0]) + q.y*(-q.w*P[39] + q.x*P[46] - q.z*P[32] + (-b.x + w.x)*P[25] + (-b.y + w.y)*P[4] + (b.z - w.z)*P[11] + P[18]) - q.z*P[31] + (-b.x + w.x)*P[24] + (b.x - w.x)*(-q.w*P[37] + q.x*P[44] - q.z*P[30] + (-b.x + w.x)*P[23] + (-b.y + w.y)*P[2] + (b.z - w.z)*P[9] + P[16]) + (-b.y + w.y)*(-q.w*P[36] + q.x*P[43] - q.z*P[29] + (-b.x + w.x)*P[22] + (-b.y + w.y)*P[1] + (b.z - w.z)*P[8] + P[15]) + (-b.y + w.y)*P[3] + (-b.z + w.z)*(-q.w*P[35] + q.x*P[42] - q.z*P[28] + (-b.x + w.x)*P[21] + (-b.y + w.y)*P[0] + (b.z - w.z)*P[7] + P[14]) + (b.z - w.z)*P[10] + P[17];
        p_pred[18] = -q.w*P[39] + q.x*P[46] - q.z*P[32] + (-b.x + w.x)*P[25] + (-b.y + w.y)*P[4] + (b.z - w.z)*P[11] + P[18];
        p_pred[19] = -q.w*P[40] + q.x*P[47] - q.z*P[33] + (-b.x + w.x)*P[26] + (-b.y + w.y)*P[5] + (b.z - w.z)*P[12] + P[19];
        p_pred[20] = -q.w*P[41] + q.x*P[48] - q.z*P[34] + (-b.x + w.x)*P[27] + (-b.y + w.y)*P[6] + (b.z - w.z)*P[13] + P[20];
        p_pred[21] = -q.w*P[42] - q.x*(q.w*W[6] + q.x*W[3] - q.y*W[0]) + q.x*(-q.w*P[46] - q.x*P[39] + q.y*P[32] + (b.x - w.x)*P[18] + (-b.y + w.y)*P[11] + (-b.z + w.z)*P[4] + P[25]) - q.x*P[35] - q.y*(q.w*W[7] + q.x*W[4] - q.y*W[1]) + q.y*(-q.w*P[47] - q.x*P[40] + q.y*P[33] + (b.x - w.x)*P[19] + (-b.y + w.y)*P[12] + (-b.z + w.z)*P[5] + P[26]) + q.y*P[28] - q.z*(q.w*W[8] + q.x*W[5] - q.y*W[2]) + q.z*(-q.w*P[48] - q.x*P[41] + q.y*P[34] + (b.x - w.x)*P[20] + (-b.y + w.y)*P[13] + (-b.z + w.z)*P[6] + P[27]) + (b.x - w.x)*(-q.w*P[43] - q.x*P[36] + q.y*P[29] + (b.x - w.x)*P[15] + (-b.y + w.y)*P[8] + (-b.z + w.z)*P[1] + P[22]) + (b.x - w.x)*P[14] + (-b.y + w.y)*P[7] + (b.y - w.y)*(-q.w*P[44] - q.x*P[37] + q.y*P[30] + (b.x - w.x)*P[16] + (-b.y + w.y)*P[9] + (-b.z + w.z)*P[2] + P[23]) + (-b.z + w.z)*P[0] + (b.z - w.z)*(-q.w*P[45] - q.x*P[38] + q.y*P[31] + (b.x - w.x)*P[17] + (-b.y + w.y)*P[10] + (-b.z + w.z)*P[3] + P[24]) + P[21];
        p_pred[22] = q.w*(q.w*W[6] + q.x*W[3] - q.y*W[0]) - q.w*(-q.w*P[46] - q.x*P[39] + q.y*P[32] + (b.x - w.x)*P[18] + (-b.y + w.y)*P[11] + (-b.z + w.z)*P[4] + P[25]) - q.w*P[43] - q.x*P[36] + q.y*(q.w*W[8] + q.x*W[5] - q.y*W[2]) - q.y*(-q.w*P[48] - q.x*P[41] + q.y*P[34] + (b.x - w.x)*P[20] + (-b.y + w.y)*P[13] + (-b.z + w.z)*P[6] + P[27]) + q.y*P[29] - q.z*(q.w*W[7] + q.x*W[4] - q.y*W[1]) + q.z*(-q.w*P[47] - q.x*P[40] + q.y*P[33] + (b.x - w.x)*P[19] + (-b.y + w.y)*P[12] + (-b.z + w.z)*P[5] + P[26]) + (-b.x + w.x)*(-q.w*P[42] - q.x*P[35] + q.y*P[28] + (b.x - w.x)*P[14] + (-b.y + w.y)*P[7] + (-b.z + w.z)*P[0] + P[21]) + (b.x - w.x)*P[15] + (-b.y + w.y)*P[8] + (b.y - w.y)*(-q.w*P[45] - q.x*P[38] + q.y*P[31] + (b.x - w.x)*P[17] + (-b.y + w.y)*P[10] + (-b.z + w.z)*P[3] + P[24]) + (-b.z + w.z)*(-q.w*P[44] - q.x*P[37] + q.y*P[30] + (b.x - w.x)*P[16] + (-b.y + w.y)*P[9] + (-b.z + w.z)*P[2] + P[23]) + (-b.z + w.z)*P[1] + P[22];
        p_pred[23] = q.w*(q.w*W[7] + q.x*W[4] - q.y*W[1]) - q.w*(-q.w*P[47] - q.x*P[40] + q.y*P[33] + (b.x - w.x)*P[19] + (-b.y + w.y)*P[12] + (-b.z + w.z)*P[5] + P[26]) - q.w*P[44] - q.x*(q.w*W[8] + q.x*W[5] - q.y*W[2]) + q.x*(-q.w*P[48] - q.x*P[41] + q.y*P[34] + (b.x - w.x)*P[20] + (-b.y + w.y)*P[13] + (-b.z + w.z)*P[6] + P[27]) - q.x*P[37] + q.y*P[30] + q.z*(q.w*W[6] + q.x*W[3] - q.y*W[0]) - q.z*(-q.w*P[46] - q.x*P[39] + q.y*P[32] + (b.x - w.x)*P[18] + (-b.y + w.y)*P[11] + (-b.z + w.z)*P[4] + P[25]) + (-b.x + w.x)*(-q.w*P[45] - q.x*P[38] + q.y*P[31] + (b.x - w.x)*P[17] + (-b.y + w.y)*P[10] + (-b.z + w.z)*P[3] + P[24]) + (b.x - w.x)*P[16] + (-b.y + w.y)*(-q.w*P[42] - q.x*P[35] + q.y*P[28] + (b.x - w.x)*P[14] + (-b.y + w.y)*P[7] + (-b.z + w.z)*P[0] + P[21]) + (-b.y + w.y)*P[9] + (-b.z + w.z)*P[2] + (b.z - w.z)*(-q.w*P[43] - q.x*P[36] + q.y*P[29] + (b.x - w.x)*P[15] + (-b.y + w.y)*P[8] + (-b.z + w.z)*P[1] + P[22]) + P[23];
        p_pred[24] = q.w*(q.w*W[8] + q.x*W[5] - q.y*W[2]) - q.w*(-q.w*P[48] - q.x*P[41] + q.y*P[34] + (b.x - w.x)*P[20] + (-b.y + w.y)*P[13] + (-b.z + w.z)*P[6] + P[27]) - q.w*P[45] + q.x*(q.w*W[7] + q.x*W[4] - q.y*W[1]) - q.x*(-q.w*P[47] - q.x*P[40] + q.y*P[33] + (b.x - w.x)*P[19] + (-b.y + w.y)*P[12] + (-b.z + w.z)*P[5] + P[26]) - q.x*P[38] - q.y*(q.w*W[6] + q.x*W[3] - q.y*W[0]) + q.y*(-q.w*P[46] - q.x*P[39] + q.y*P[32] + (b.x - w.x)*P[18] + (-b.y + w.y)*P[11] + (-b.z + w.z)*P[4] + P[25]) + q.y*P[31] + (b.x - w.x)*(-q.w*P[44] - q.x*P[37] + q.y*P[30] + (b.x - w.x)*P[16] + (-b.y + w.y)*P[9] + (-b.z + w.z)*P[2] + P[23]) + (b.x - w.x)*P[17] + (-b.y + w.y)*(-q.w*P[43] - q.x*P[36] + q.y*P[29] + (b.x - w.x)*P[15] + (-b.y + w.y)*P[8] + (-b.z + w.z)*P[1] + P[22]) + (-b.y + w.y)*P[10] + (-b.z + w.z)*(-q.w*P[42] - q.x*P[35] + q.y*P[28] + (b.x - w.x)*P[14] + (-b.y + w.y)*P[7] + (-b.z + w.z)*P[0] + P[21]) + (-b.z + w.z)*P[3] + P[24];
        p_pred[25] = -q.w*P[46] - q.x*P[39] + q.y*P[32] + (b.x - w.x)*P[18] + (-b.y + w.y)*P[11] + (-b.z + w.z)*P[4] + P[25];
        p_pred[26] = -q.w*P[47] - q.x*P[40] + q.y*P[33] + (b.x - w.x)*P[19] + (-b.y + w.y)*P[12] + (-b.z + w.z)*P[5] + P[26];
        p_pred[27] = -q.w*P[48] - q.x*P[41] + q.y*P[34] + (b.x - w.x)*P[20] + (-b.y + w.y)*P[13] + (-b.z + w.z)*P[6] + P[27];
        p_pred[28] = q.x*P[32] + q.y*P[33] + q.z*P[34] + (b.x - w.x)*P[29] + (b.y - w.y)*P[30] + (b.z - w.z)*P[31] + P[28];
        p_pred[29] = -q.w*P[32] - q.y*P[34] + q.z*P[33] + (-b.x + w.x)*P[28] + (b.y - w.y)*P[31] + (-b.z + w.z)*P[30] + P[29];
        p_pred[30] = -q.w*P[33] + q.x*P[34] - q.z*P[32] + (-b.x + w.x)*P[31] + (-b.y + w.y)*P[28] + (b.z - w.z)*P[29] + P[30];
        p_pred[31] = -q.w*P[34] - q.x*P[33] + q.y*P[32] + (b.x - w.x)*P[30] + (-b.y + w.y)*P[29] + (-b.z + w.z)*P[28] + P[31];
        p_pred[32] = P[32];
        p_pred[33] = P[33];
        p_pred[34] = P[34];
        p_pred[35] = q.x*P[39] + q.y*P[40] + q.z*P[41] + (b.x - w.x)*P[36] + (b.y - w.y)*P[37] + (b.z - w.z)*P[38] + P[35];
        p_pred[36] = -q.w*P[39] - q.y*P[41] + q.z*P[40] + (-b.x + w.x)*P[35] + (b.y - w.y)*P[38] + (-b.z + w.z)*P[37] + P[36];
        p_pred[37] = -q.w*P[40] + q.x*P[41] - q.z*P[39] + (-b.x + w.x)*P[38] + (-b.y + w.y)*P[35] + (b.z - w.z)*P[36] + P[37];
        p_pred[38] = -q.w*P[41] - q.x*P[40] + q.y*P[39] + (b.x - w.x)*P[37] + (-b.y + w.y)*P[36] + (-b.z + w.z)*P[35] + P[38];
        p_pred[39] = P[39];
        p_pred[40] = P[40];
        p_pred[41] = P[41];
        p_pred[42] = q.x*P[46] + q.y*P[47] + q.z*P[48] + (b.x - w.x)*P[43] + (b.y - w.y)*P[44] + (b.z - w.z)*P[45] + P[42];
        p_pred[43] = -q.w*P[46] - q.y*P[48] + q.z*P[47] + (-b.x + w.x)*P[42] + (b.y - w.y)*P[45] + (-b.z + w.z)*P[44] + P[43];
        p_pred[44] = -q.w*P[47] + q.x*P[48] - q.z*P[46] + (-b.x + w.x)*P[45] + (-b.y + w.y)*P[42] + (b.z - w.z)*P[43] + P[44];
        p_pred[45] = -q.w*P[48] - q.x*P[47] + q.y*P[46] + (b.x - w.x)*P[44] + (-b.y + w.y)*P[43] + (-b.z + w.z)*P[42] + P[45];
        p_pred[46] = P[46];
        p_pred[47] = P[47];
        p_pred[48] = P[48];

    }

    

};