 /**
 * @file lte_sec_wrapper.h
 * @brief Definitions of existing baseband firmware LTE security functions
 * 
 * Specific to Asus Padfone Infinity 2 (A86), firmware version "M3.12.15-A86_1032"
 * 
 * @author Carsten Bruns (carst.bruns@gmx.de)
 */

#ifndef __LTE_SEC_WRAPPER_H
#define __LTE_SEC_WRAPPER_H

ADDRESS(0x9F1A954) int mutils_security_stream_cipher(unsigned char technology, unsigned int algo, unsigned char* key, unsigned char* in_msg_ptr, unsigned short in_msg_byte_len, unsigned char* out_msg_ptr, unsigned char bearer, unsigned int count);
ADDRESS(0x9F1A408) int mutils_security_stream_decipher(unsigned char technology, unsigned int algo, unsigned char* key, unsigned char* in_msg_ptr, unsigned short in_msg_byte_len, unsigned char* out_msg_ptr, unsigned char bearer, unsigned int count);
ADDRESS(0x9F1AE94) int mutils_security_stream_compute_integrity_maci(unsigned char technology, unsigned int algo, unsigned char* key, unsigned char* in_msg_ptr, unsigned short in_msg_byte_len, unsigned int* maci_ptr, unsigned char bearer, unsigned int fresh, unsigned int count, unsigned char direction);

ADDRESS(0x872A53C) int lte_security_stream_cipher(unsigned int algo, unsigned char* key, unsigned char* in_msg_ptr, unsigned short in_msg_byte_len, unsigned char* out_msg_ptr, unsigned char bearer, unsigned int count);
ADDRESS(0x872A4A0) int lte_security_stream_decipher(unsigned int algo, unsigned char* key, unsigned char* in_msg_ptr, unsigned short in_msg_byte_len, unsigned char* out_msg_ptr, unsigned char bearer, unsigned int count);
ADDRESS(0x872A898) int lte_security_stream_compute_integrity_maci(unsigned int algo, unsigned char* key, unsigned char* in_msg_ptr, unsigned short in_msg_byte_len, unsigned int* maci_ptr, unsigned char bearer, unsigned int count, unsigned char direction);

ADDRESS(0x872A5D8) int lte_security_generate_key(unsigned char* in_key, unsigned char* in_string_ptr, unsigned char* in_string_len, unsigned char* out_key);
ADDRESS(0x872A7B4) int lte_security_generate_algorithm_key(unsigned char* in_key, unsigned int algorithm_distinguisher, unsigned char algorithm_type, unsigned char* out_key_ptr);

ADDRESS(0x9F0F454) void a2_ul_sec_process_write_keys_req(unsigned char first_key_index, unsigned char* first_key, unsigned char second_key_index, unsigned char* second_key, unsigned char third_key_index, unsigned char* third_key);

#endif
