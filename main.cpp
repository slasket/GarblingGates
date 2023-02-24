

#include <boost/lambda/lambda.hpp>
#include <iostream>
#include <cryptoTools/Common/Defines.h>
#include "bloodcompatibility.h"
#include "util/otUtil/otUtil.h"
#include "util/OTUtil/OT.h"
#include "util/circuitParser.h"
#include <bitset>
#include <cstdio>


#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>


using namespace std;

#include <openssl/sha.h>
#include <openssl/aes.h>
#include <openssl/evp.h>


void testBaseOT(int v, int k ,int l, int elgamalKeySize){
    cout<< "#Testing baseOTs from "<< v << " to " << k<< " Amount of OTs"<<endl;

    for (int j = v; j <= k; j=j*2) {
        vector<tuple<vector<oc::u64>,vector<oc::u64>>> senderPairs(j);
        vector<oc::u64> selectionBits(j);
        senderPairs = OT::genKAmountOfSelectionStrings(j, 128);

        selectionBits = otUtil::genBitsNonCrypto(j);

        cout<< "Initial OT with "<< j<< " OTs " << endl;

        //initial OT phase
        //cout<< "Starting initial OT phase" << endl;
        auto result = OT::BaseOTTest(elgamalKeySize, j, senderPairs, selectionBits);
        cout<< "done with "<< j<< " OTs " << endl;

        string choicebits;
        int correctcounter = 0;
        int incorrectcounter = 0;
        int zeroes = 0;
        int ones = 0;
        for (int i = 0; i < result->size(); ++i) {
            int choicebit = otUtil::findithBit(selectionBits, i);
            choicebits += to_string(choicebit);
            if(choicebit == 0){
                if(otUtil::printBitsetofVectorofUints(result[i]) == otUtil::printBitsetofVectorofUints(get<0>(senderPairs[i])) ){
                    //cout<<"res: "<<result[i]<<endl;
                    //cout<<"par: "<<otUtil::str2bitstr(get<0>(senderPairs[i]))<<endl;
                    correctcounter++;
                    zeroes++;
                } else {
                    //cout<<"res: "<<result[i]<<endl;
                    //cout<<"par: "<<otUtil::str2bitstr(get<0>(senderPairs[i]))<<endl;
                    incorrectcounter++;
                }
            }else{
                if(otUtil::printBitsetofVectorofUints(result[i]) == otUtil::printBitsetofVectorofUints(get<1>(senderPairs[i])) ){
                    //cout<<"res: "<<result[i]<<endl;
                    //cout<<"par: "<<otUtil::str2bitstr(get<1>(senderPairs[i]))<<endl;
                    correctcounter++;
                    ones++;
                } else {
                    //cout<<"res: "<<result[i]<<endl;
                    //cout<<"par: "<<otUtil::str2bitstr(get<1>(senderPairs[i]))<<endl;
                    incorrectcounter++;
                }
            }
        }
        //reverse findIntchoicebits
        //string findIntchoicebitsReversed;
        //for (int i = 0; i < findIntchoicebits.length(); ++i) {
        //    findIntchoicebitsReversed += findIntchoicebits[findIntchoicebits.length()-1-i];
        //}
        //cout << "findIn Rev: " << findIntchoicebitsReversed << endl;
        //count where choicebits and findIntchoicebits differ
        cout << "correct: " << correctcounter << endl;
        cout << "incorrect: " << incorrectcounter << endl;
        cout<<"zeroes: "<<zeroes<<endl;
        cout<<"ones: "<<ones<<endl;
    }
}


string sha256(const string str){
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, str.c_str(), str.size());
    SHA256_Final(hash, &sha256);
    stringstream ss;
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++){
        ss << hex << setw(2) << setfill('0') << (int)hash[i];
    }
    return ss.str();
}

/**
 * Create a 256 bit key and IV using the supplied key_data. salt can be added for taste.
 * Fills in the encryption and decryption ctx objects and returns 0 on success
 **/
int aes_init(unsigned char *key_data, int key_data_len, unsigned char *salt, EVP_CIPHER_CTX *e_ctx,
             EVP_CIPHER_CTX *d_ctx)
{
    int i, nrounds = 5;
    unsigned char key[32], iv[32];

    /*
     * Gen key & IV for AES 256 CTR mode. A SHA1 digest is used to hash the supplied key material.
     * nrounds is the number of times the we hash the material. More rounds are more secure but
     * slower.
     */
    i = EVP_BytesToKey(EVP_aes_256_cbc(), EVP_sha1(), salt, key_data, key_data_len, nrounds, key, iv);
    if (i != 32) {
        printf("Key size is %d bits - should be 256 bits\n", i);
        return -1;
    }

    EVP_CIPHER_CTX_init(e_ctx);
    EVP_EncryptInit_ex(e_ctx, EVP_aes_256_cbc(), NULL, key, iv);
    EVP_CIPHER_CTX_init(d_ctx);
    EVP_DecryptInit_ex(d_ctx, EVP_aes_256_cbc(), NULL, key, iv);

    return 0;
}

/*
 * Encrypt *len bytes of data
 * All data going in & out is considered binary (unsigned char[])
 */
unsigned char *aes_encrypt(EVP_CIPHER_CTX *e, unsigned char *plaintext, int *len)
{
    /* max ciphertext len for a n bytes of plaintext is n + AES_BLOCK_SIZE -1 bytes */
    int c_len = *len + AES_BLOCK_SIZE, f_len = 0;
    unsigned char *ciphertext = static_cast<unsigned char *>(malloc(c_len));

    /* allows reusing of 'e' for multiple encryption cycles */
    EVP_EncryptInit_ex(e, NULL, NULL, NULL, NULL);

    /* update ciphertext, c_len is filled with the length of ciphertext generated,
      *len is the size of plaintext in bytes */
    EVP_EncryptUpdate(e, ciphertext, &c_len, plaintext, *len);

    /* update ciphertext with the final remaining bytes */
    EVP_EncryptFinal_ex(e, ciphertext+c_len, &f_len);

    *len = c_len + f_len;
    return ciphertext;
}
/*
 * Decrypt *len bytes of ciphertext
 */
unsigned char *aes_decrypt(EVP_CIPHER_CTX *e, unsigned char *ciphertext, int *len)
{
    /* plaintext will always be equal to or lesser than length of ciphertext*/
    int p_len = *len, f_len = 0;
    unsigned char *plaintext = static_cast<unsigned char *>(malloc(p_len));

    EVP_DecryptInit_ex(e, NULL, NULL, NULL, NULL);
    EVP_DecryptUpdate(e, plaintext, &p_len, ciphertext, *len);
    EVP_DecryptFinal_ex(e, plaintext+p_len, &f_len);

    *len = p_len + f_len;
    return plaintext;
}

int mainAES();

void test_hash_variable();

//perform variable output length hash
vector<uint64_t> hash_variable(const std::string& input, int output_length_bits = 128)
{
    size_t output_length_bytes = output_length_bits / 8;
    EVP_MD_CTX* ctx = EVP_MD_CTX_create();
    EVP_DigestInit_ex(ctx, EVP_shake256(), NULL);
    EVP_DigestUpdate(ctx, input.c_str(), input.size());
    std::string output(output_length_bytes, '\0');
    EVP_DigestFinalXOF(ctx, reinterpret_cast<unsigned char *>(&output[0]), output_length_bytes);
    EVP_MD_CTX_destroy(ctx);
    //convert output to vector<uint64_t>
    vector<uint64_t> output_vector;
    for (int i = 0; i < output_length_bytes; i+=8) {
        uint64_t temp = 0;
        for (int j = 0; j < 8; ++j) {
            temp += (uint64_t)output[i+j] << (j*8);
        }
        output_vector.push_back(temp);
    }
    return output_vector;
}

int main() {
    //cout << sha256("1234567890_1") << endl;
    //cout << sha256("1234567890_2") << endl;
    //cout << sha256("1234567890_3") << endl;
    //cout << sha256("1234567890_4") << endl;

    //int aes =  mainAES();

    //test hash_variable with size 32, 64, 128, 256, 512, 1024
    test_hash_variable();

    //string input = "1234567890_1";
    //vector<uint64_t> output = hash_variable(input);
    //cout << "output size: " << output.size() * 64 << endl;
    //for (int i = 0; i < output.size(); ++i) {
    //    cout << output[i] << " ";
    //}
    //cout << endl;

    return 0;
}

void test_hash_variable() {
    string input = "1234567890_1";
    for (int i = 0; i < 6; i++) {
        vector<uint64_t> output = hash_variable(input, ::pow(2, i+7));
        cout << "output size: " << output.size() * 64 << endl;
        for (int i = 0; i < output.size(); ++i) {
            cout << output[i] << " ";
        }
        cout << endl;
    }
}

int mainAES() {
    unsigned char xd[20] ="hello world";
    /* "opaque" encryption, decryption ctx structures that libcrypto uses to record
        status of enc/dec operations */
    EVP_CIPHER_CTX* en = EVP_CIPHER_CTX_new();
    EVP_CIPHER_CTX* de = EVP_CIPHER_CTX_new();

    /* 8 bytes to salt the key_data during key generation. This is an example of
       compiled in salt. We just read the bit pattern created by these two 4 byte
       integers on the stack as 64 bits of contigous salt material -
       ofcourse this only works if sizeof(int) >= 4 */
    unsigned int salt[] = {12345, 54321};
    unsigned char *key_data;
    int key_data_len, i;
    char *input[] = {"012345678901234567890123456789010123456789012345678901234567890101234567890123456789012345678901012345678901234567890123456789010123456789012345678901234567890101234567890123456789012345678901012345678901234567890123456789010123456789012345678901234567890101234567890123456789012345678901012345678901234567890123456789010123456789012345678901234567890101234567890123456789012345678901012345678901234567890123456789010123456789012345678901234567890101234567890123456789012345678901012345678901234567890123456789010123456789012345678901234567890101234567890123456789012345678901", "abcd", "this is a test", "this is a bigger test",
                     "\nWho are you ?\nI am the 'Doctor'.\n'Doctor' who ?\nPrecisely!",
                     NULL};

    key_data = (unsigned char *)"01234567890123456789012345678901";
    key_data_len = strlen((char *)key_data);

    /* gen key and iv. init the cipher ctx object */
    if (aes_init(key_data, key_data_len, (unsigned char *)&salt, en, de)) {
        printf("Couldn't initialize AES cipher\n");
        return -1;
    }

    /* encrypt and decrypt each input string and compare with the original */
    for (i = 0; input[i]; i++) {
        char *plaintext;
        unsigned char *ciphertext;
        int olen, len;

        /* The enc/dec functions deal with binary data and not C strings. strlen() will
           return length of the string without counting the '\0' string marker. We always
           pass in the marker byte to the encrypt/decrypt functions so that after decryption
           we end up with a legal C string */
        olen = len = 256;
        //olen = len = strlen(input[i])+1;

        ciphertext = aes_encrypt(en, (unsigned char *)input[i], &len);
        //print the ciphertext
        for (int j = 0; j < len; j++) {
            printf("%02x", ciphertext[j]);
        }
        printf("\n");
        plaintext = (char *)aes_decrypt(de, ciphertext, &len);

        if (strncmp(plaintext, input[i], olen))
            printf("FAIL: enc/dec failed for \"%s\"\n", input[i]);
        else
            printf("OK: enc/dec ok for \"%s\"\n", plaintext);

        free(ciphertext);
        free(plaintext);
    }

    EVP_CIPHER_CTX_free(en);
    EVP_CIPHER_CTX_free(de);
    return 0;
}

//util::printCircuit("../tests/circuits/BloodComp.txt");
//testBaseOT(64,512,256,2048);
//bloodcompatibility bc;
//bc.testAllCombinations();
