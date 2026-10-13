extern "C"{
    #include "../../trd/openssl/include/openssl/evp.h"
    #include "../../trd/openssl/include/openssl/rand.h"
    #include "../../trd/openssl/include/openssl/bio.h"
    #include "../../trd/openssl/include/openssl/buffer.h"
    #include "../../trd/tz_audio_player/include/tz_pcm_audio.h"
    #include "../../trd/tz_audio_player/include/tz_audio_player.h"
}
#include "http_get_online_res.h"
#include "global_http_tool.h"
using namespace std;

// 辅助函数实现 -------------------------------------------------
vector<unsigned char> hexToBytes(const string& hex) {
    vector<unsigned char> bytes;
    for (size_t i = 0; i < hex.length(); i += 2) {
        string byteStr = hex.substr(i, 2);
        unsigned char byte = (unsigned char)strtol(byteStr.c_str(), NULL, 16);
        bytes.push_back(byte);
    }
    return bytes;
}

string base64Encode(const vector<unsigned char>& data) {
    std::unique_ptr<BIO, decltype(&::BIO_free_all)> b64(BIO_new(BIO_f_base64()), BIO_free_all);
    BIO_set_flags(b64.get(), BIO_FLAGS_BASE64_NO_NL);
    BIO* sink = BIO_new(BIO_s_mem());
    BIO_push(b64.get(), sink);
    BIO_write(b64.get(), data.data(), static_cast<int>(data.size()));
    BIO_flush(b64.get());
    char* encoded;
    const long len64 = BIO_get_mem_data(sink, &encoded);
    return std::string(encoded, len64);
}

void online_res_play_audio_by_url(const string& url) {
    tz_play_audio(url.c_str(),0,-1,true);
}

string urlEncode(const string& s) {
    ostringstream escaped;
    escaped.fill('0');
    escaped << hex;
    
    for (char c : s) {
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            escaped << c;
        } else {
            escaped << '%' << setw(2) << int(static_cast<unsigned char>(c));
        }
    }
    return escaped.str();
}

std::string get_online_res_req_sign(){
    std::string deviceId = get_device_id();
    std::string timestamp = get_current_time_ms();
    std::string jsonData = R"({"deviceId":")" + deviceId + R"(","timestamp":)" + timestamp + "}";
    cout<<"jsonData:"<<jsonData<<endl;

    constexpr int GCM_IV_LENGTH = 12;
    constexpr int GCM_TAG_LENGTH = 16;
    constexpr int AES_KEY_LENGTH = 32;

    const unsigned char key[AES_KEY_LENGTH] = {
        0xf2,0xcc,0x49,0x34,0xad,0xfc,0xa8,0xc8,0xd8,0xbe,0x0a,0xcf,0x8d,0x70,0x6b,0xb1,
        0x47,0x65,0xbe,0xb2,0xc8,0x05,0x07,0x6d,0xfd,0x07,0x1e,0x32,0x37,0x12,0xa4,0xf2
    };

    std::vector<unsigned char> iv(GCM_IV_LENGTH);
    if (RAND_bytes(iv.data(), GCM_IV_LENGTH) != 1) {
        throw std::runtime_error("IV generation failed");
    }

    std::unique_ptr<EVP_CIPHER_CTX, decltype(&::EVP_CIPHER_CTX_free)> ctx(
        EVP_CIPHER_CTX_new(), ::EVP_CIPHER_CTX_free);
    if (!ctx) throw std::runtime_error("Context creation failed");

    if (1 != EVP_EncryptInit_ex(ctx.get(), EVP_aes_256_gcm(), nullptr, nullptr, nullptr) ||
        1 != EVP_CIPHER_CTX_ctrl(ctx.get(), EVP_CTRL_GCM_SET_IVLEN, GCM_IV_LENGTH, nullptr) ||
        1 != EVP_EncryptInit_ex(ctx.get(), nullptr, nullptr, key, iv.data())) {
        throw std::runtime_error("Cipher initialization failed");
    }

    std::vector<unsigned char> ciphertext(jsonData.size() + GCM_TAG_LENGTH);
    int len = 0, ciphertext_len = 0;
    
    if (1 != EVP_EncryptUpdate(ctx.get(), ciphertext.data(), &len,
                             reinterpret_cast<const unsigned char*>(jsonData.data()),
                             static_cast<int>(jsonData.size()))) {
        throw std::runtime_error("Encryption failed");
    }
    ciphertext_len = len;

    unsigned char* final_pos = ciphertext.data() + len;
    if (1 != EVP_EncryptFinal_ex(ctx.get(), final_pos, &len)) {
        throw std::runtime_error("Finalization failed");
    }
    if (len > 0) {
        ciphertext_len += len;
    }

    std::vector<unsigned char> tag(GCM_TAG_LENGTH);
    if (1 != EVP_CIPHER_CTX_ctrl(ctx.get(), EVP_CTRL_GCM_GET_TAG, GCM_TAG_LENGTH, tag.data())) {
        throw std::runtime_error("Tag retrieval failed");
    }

    std::vector<unsigned char> combined;
    combined.reserve(GCM_IV_LENGTH + ciphertext_len + GCM_TAG_LENGTH);
    combined.insert(combined.end(), iv.begin(), iv.end());
    combined.insert(combined.end(), ciphertext.begin(), ciphertext.begin() + ciphertext_len);
    combined.insert(combined.end(), tag.begin(), tag.end());

    
    
    std::string encoded = base64Encode(combined);
    std::cout << "Base64 Encoded: " << encoded << std::endl;
    return url_encode(encoded);
}

void get_online_res_req(std::string path,int pageSize,std::unique_ptr<GetResResponse>& res,
                    std::function<void()> callback){
    GetResRequest req;
    req.parentPath = path;
    req.sign = get_online_res_req_sign();
    req.vendor = "hntz";
    req.product = "dzxsz";
    req.position = 0;
    req.pageSize = pageSize;
    send_request_async<GetResRequest,GetResResponse>
                    (req,send_GetRes_request,res,callback);
}

void get_online_media_res_req(std::string path,int pageSize,std::unique_ptr<GetResResponse>& res,
    std::function<void()> callback){
    GetResRequest req;
    req.parentPath = path;
    req.sign = get_online_res_req_sign();
    req.vendor = "hntz";
    req.product = "dzxsz";
    req.position = 0;
    req.pageSize = pageSize;
    send_request_async<GetResRequest,GetResResponse>
                        (req,send_GetAllMediaRes_request,res,callback);
}