/**
 * @file lte_sec.c
 * @brief LTE security/keys extracting (over QMI) patches
 *
 * @author Carsten Bruns (carst.bruns@gmx.de)
 */

#include FW_WRAPPER
#include "../../common/seemoo_qmi/qmi_message_structs.h"
#include "lte_sec.h"

#define is_registered(option) ((registration_flags & option) == option)

/**
 * @brief registration flags specifying what indications with which content should be sent
 */
typedef enum {
    GENERATED_KEYS_MASK = ((1 << 0) | (1 << 1)), /* mask for all generated keys flags */
    GENERATED_KEYS = (1 << 0), /* generated keys */
    GENERATED_KEYS_INPUT = ((1 << 0) | (1 << 1)), /* include input data (in_key, in_string, in_string_length) for generated keys */
    
    GENERATED_ALGO_KEYS_MASK = ((1 << 2) | (1 << 3)), /* mask for all generated algorithm keys flags */
    GENERATED_ALGO_KEYS = (1 << 2), /* generated algorithm keys (includes KeyUse, KeyAlgorithm)) */
    GENERATED_ALGO_KEYS_INPUT = ((1 << 2) | (1 << 3)), /* include input data (in_key)  */
    
    CIPHER_CALLS_MASK = ((1 << 4) | (1 << 5) | (1 << 6) | (1 << 7)), /* mask for all cipher call flags */
    CIPHER_CALLS = (1 << 4), /* cipher calls (includes used algorithm, bearer, count, msg_length) */
    CIPHER_CALLS_KEY = ((1 << 4) | (1 << 5)), /* include used key */
    CIPHER_CALLS_IN_MSG = ((1 << 4) | (1 << 6)), /* include input message */
    CIPHER_CALLS_OUT_MSG = ((1 << 4) | (1 << 7)), /* include ciphered message */
    
    DECIPHER_CALLS_MASK = ((1 << 8) | (1 << 9) | (1 << 10) | (1 << 11)), /* mask for all decipher call flags */
    DECIPHER_CALLS = (1 << 8), /* decipher calls (includes used algorithm, bearer, count, msg_length) */
    DECIPHER_CALLS_KEY = ((1 << 8) | (1 << 9)), /* include used key */
    DECIPHER_CALLS_IN_MSG = ((1 << 8) | (1 << 10)), /* include input message */
    DECIPHER_CALLS_OUT_MSG = ((1 << 8) | (1 << 11)), /* include deciphered message */
    
    MACI_CALLS_MASK = ((1 << 12) | (1 << 13) | (1 << 14) | (1 << 15)), /* mask for all MAC generation call flags */
    MACI_CALLS = (1 << 12), /* MAC generation calls (includes used algorithm, bearer, count, msg_length, direction) */
    MACI_CALLS_KEY = ((1 << 12) | (1 << 13)), /* include used key */
    MACI_CALLS_IN_MSG = ((1 << 12) | (1 << 14)), /* include input message */
    MACI_CALLS_MAC = ((1 << 12) | (1 << 15)) /* include generated MAC */
} registration_flags_t;

/* LTE security client pointer, NULL when no client is set */
void* lte_sec_svc_client;
/* registration options, OR combination of elements of enumeration registration_flags_t */
unsigned int registration_flags;

/* structure used to build QMI indications */
test_data_ind_msg_v01 indication_buf;

void lte_sec_prepare_ind(unsigned int flags_mask) {
    memset(&indication_buf, 0, sizeof(test_data_ind_msg_v01));
    unsigned int* data32 = (unsigned int*)(indication_buf.data);
    *data32 = LTE_SEC_SVC_ID;
    *(data32 + 1) = registration_flags & flags_mask;
}

__attribute__ ((overwrite ("lte_security_generate_key")))
int lte_security_generate_key_hook(unsigned char* in_key, unsigned char* in_string_ptr, unsigned char* in_string_len, unsigned char* out_key) {
    int res = lte_security_generate_key_fw_org(in_key, in_string_ptr, in_string_len, out_key);
    
    if ((lte_sec_svc_client != 0) && is_registered(GENERATED_KEYS)) {        
        lte_sec_prepare_ind(GENERATED_KEYS_MASK);
        
        memcpy(indication_buf.data + 8, out_key, 32);
        
        int pos = 8 + 32;
        if (is_registered(GENERATED_KEYS_INPUT)) {
            memcpy(indication_buf.data + pos, in_key, 32);
            pos += 32;
            *(indication_buf.data + pos++) = *in_string_len;
            memcpy(indication_buf.data + pos, in_string_ptr, *in_string_len);
            pos += *in_string_len;
        }
        
        indication_buf.data_len = pos;
        
        qmi_csi_send_ind(lte_sec_svc_client, QMI_TEST_DATA_IND_V01, &indication_buf, sizeof(test_data_ind_msg_v01));
    }
    
    return res;
}

__attribute__ ((overwrite ("lte_security_generate_algorithm_key")))
int lte_security_generate_algorithm_key_hook(unsigned char* in_key, unsigned int algorithm_distinguisher, unsigned char algorithm_type, unsigned char* out_key_ptr) {
    int res = lte_security_generate_algorithm_key_fw_org(in_key, algorithm_distinguisher, algorithm_type, out_key_ptr);
    
    if ((lte_sec_svc_client != 0) && is_registered(GENERATED_ALGO_KEYS)) {        
        lte_sec_prepare_ind(GENERATED_ALGO_KEYS_MASK);
        
        memcpy(indication_buf.data + 8, out_key_ptr, 16);
        *(indication_buf.data + 24) = algorithm_distinguisher;
        *(indication_buf.data + 25) = algorithm_type;
        *(indication_buf.data + 26) = 0;
        *(indication_buf.data + 27) = 0;
        
        int pos = 8 + 16 + 4;
        if (is_registered(GENERATED_ALGO_KEYS_INPUT)) {
            memcpy(indication_buf.data + pos, in_key, 32);
            pos += 32;
        }
        
        indication_buf.data_len = pos;
        
        qmi_csi_send_ind(lte_sec_svc_client, QMI_TEST_DATA_IND_V01, &indication_buf, sizeof(test_data_ind_msg_v01));
    }
    
    return res;
}

__attribute__ ((overwrite ("mutils_security_stream_cipher")))
int mutils_security_stream_cipher_hook(unsigned char technology, unsigned int algo, unsigned char* key, unsigned char* in_msg_ptr, unsigned short in_msg_byte_len, unsigned char* out_msg_ptr, unsigned char bearer, unsigned int count) {
    int res = 0;
    
    if ((lte_sec_svc_client != 0) && (technology == 1) && is_registered(CIPHER_CALLS)) {        
        lte_sec_prepare_ind(CIPHER_CALLS_MASK);
        
        *(indication_buf.data + 8) = algo;
        *(indication_buf.data + 9) = bearer;
        *(indication_buf.data + 10) = (in_msg_byte_len & 0xFF);
        *(indication_buf.data + 11) = ((in_msg_byte_len >> 8) & 0xFF);
        *((unsigned int*)(indication_buf.data + 12)) = count;
        
        unsigned int pos = 8 + 8;
        if (is_registered(CIPHER_CALLS_KEY)) {
            memcpy(indication_buf.data + pos, key, 16);
            pos += 16;
        }
        if (is_registered(CIPHER_CALLS_IN_MSG)) {
            memcpy(indication_buf.data + pos, in_msg_ptr, in_msg_byte_len);
            pos += in_msg_byte_len;
        }
        
        //do cipher call AFTER we copied the input message (will be overwritten)
        res = mutils_security_stream_cipher_fw_org(technology, algo, key, in_msg_ptr, in_msg_byte_len, out_msg_ptr, bearer, count);
        
        if (is_registered(CIPHER_CALLS_OUT_MSG)) {
            memcpy(indication_buf.data + pos, out_msg_ptr, in_msg_byte_len);
            pos += in_msg_byte_len;
        }
        //word align message length
        if (pos % 4 != 0) {
            pos += (4 - (pos % 4));
        }
        
        indication_buf.data_len = pos;
        
        qmi_csi_send_ind(lte_sec_svc_client, QMI_TEST_DATA_IND_V01, &indication_buf, sizeof(test_data_ind_msg_v01));
    } else {
        res = mutils_security_stream_cipher_fw_org(technology, algo, key, in_msg_ptr, in_msg_byte_len, out_msg_ptr, bearer, count);
    }
    
    return res;
}

__attribute__ ((overwrite ("mutils_security_stream_decipher")))
int mutils_security_stream_decipher_hook(unsigned char technology, unsigned int algo, unsigned char* key, unsigned char* in_msg_ptr, unsigned short in_msg_byte_len, unsigned char* out_msg_ptr, unsigned char bearer, unsigned int count) {
    int res = 0;
    
    if ((lte_sec_svc_client != 0) && (technology == 1) && is_registered(DECIPHER_CALLS)) {        
        lte_sec_prepare_ind(DECIPHER_CALLS_MASK);
        
        *(indication_buf.data + 8) = algo;
        *(indication_buf.data + 9) = bearer;
        *(indication_buf.data + 10) = (in_msg_byte_len & 0xFF);
        *(indication_buf.data + 11) = ((in_msg_byte_len >> 8) & 0xFF);
        *((unsigned int*)(indication_buf.data + 12)) = count;
        
        unsigned int pos = 8 + 8;
        if (is_registered(DECIPHER_CALLS_KEY)) {
            memcpy(indication_buf.data + pos, key, 16);
            pos += 16;
        }
        if (is_registered(DECIPHER_CALLS_IN_MSG)) {
            memcpy(indication_buf.data + pos, in_msg_ptr, in_msg_byte_len);
            pos += in_msg_byte_len;
        }
        
        //do decipher call AFTER we copied the input message (will be overwritten)
        res = mutils_security_stream_decipher_fw_org(technology, algo, key, in_msg_ptr, in_msg_byte_len, out_msg_ptr, bearer, count);
        
        if (is_registered(DECIPHER_CALLS_OUT_MSG)) {
            memcpy(indication_buf.data + pos, out_msg_ptr, in_msg_byte_len);
            pos += in_msg_byte_len;
        }
        //word align message length
        if (pos % 4 != 0) {
            pos += (4 - (pos % 4));
        }
        
        indication_buf.data_len = pos;
        
        qmi_csi_send_ind(lte_sec_svc_client, QMI_TEST_DATA_IND_V01, &indication_buf, sizeof(test_data_ind_msg_v01));
    } else {
        res = mutils_security_stream_decipher_fw_org(technology, algo, key, in_msg_ptr, in_msg_byte_len, out_msg_ptr, bearer, count);
    }
    
    return res;
}

__attribute__ ((overwrite ("mutils_security_stream_compute_integrity_maci")))
int mutils_security_stream_compute_integrity_maci_hook(unsigned char technology, unsigned int algo, unsigned char* key, unsigned char* in_msg_ptr, unsigned short in_msg_byte_len, unsigned char* maci_ptr, unsigned char bearer, unsigned int fresh, unsigned int count, unsigned char direction) {
    int res = 0;
    
    if ((lte_sec_svc_client != 0) && (technology == 1) && is_registered(MACI_CALLS)) {        
        lte_sec_prepare_ind(MACI_CALLS_MASK);
        
        *(indication_buf.data + 8) = algo;
        *(indication_buf.data + 9) = bearer | ((direction & 0x1) << 7);
        *(indication_buf.data + 10) = (in_msg_byte_len & 0xFF);
        *(indication_buf.data + 11) = ((in_msg_byte_len >> 8) & 0xFF);
        *((unsigned int*)(indication_buf.data + 12)) = count;
        
        unsigned int pos = 8 + 8;
        if (is_registered(MACI_CALLS_KEY)) {
            memcpy(indication_buf.data + pos, key, 16);
            pos += 16;
        }
        if (is_registered(MACI_CALLS_IN_MSG)) {
            memcpy(indication_buf.data + pos, in_msg_ptr, in_msg_byte_len);
            pos += in_msg_byte_len;
        }
        
        //do MAC call AFTER we copied the input message (might be overwritten)
        res = mutils_security_stream_compute_integrity_maci_fw_org(technology, algo, key, in_msg_ptr, in_msg_byte_len, maci_ptr, bearer, fresh, count, direction);
        
        if (is_registered(MACI_CALLS_MAC)) {
            memcpy(indication_buf.data + pos, maci_ptr, 4);
            pos += 4;
        }
        //word align message length
        if (pos % 4 != 0) {
            pos += (4 - (pos % 4));
        }
        
        indication_buf.data_len = pos;
        
        qmi_csi_send_ind(lte_sec_svc_client, QMI_TEST_DATA_IND_V01, &indication_buf, sizeof(test_data_ind_msg_v01));
    } else {
        res = mutils_security_stream_compute_integrity_maci_fw_org(technology, algo, key, in_msg_ptr, in_msg_byte_len, maci_ptr, bearer, fresh, count, direction);
    }
    
    return res;
}

/**
 * @brief LTE security register request handler
 */
int lte_sec_svc_req(
    void* clnt_info,
    unsigned char* req_data,
    unsigned int req_data_len,
    unsigned char* resp_data) 
{    
    if (*req_data != 0) {
        lte_sec_svc_client = *((void**)clnt_info);
        *((void**)(resp_data)) = lte_sec_svc_client;
        
        registration_flags = *((unsigned int*)req_data);
        
        return 4;
    } else {
        lte_sec_svc_client = 0;
        return 0;
    }
    
    return 0;
}
