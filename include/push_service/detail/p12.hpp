//
//  async_condition_variable.hpp
//  push_service
//
//  Created by Alexander Kremer on 18/07/2013.
//  Copyright (c) 2013 godexsoft. All rights reserved.
//

#ifndef _PUSH_SERVICE_P12_HPP_
#define _PUSH_SERVICE_P12_HPP_

#include <string>
#include <stdio.h>

#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/pkcs12.h>
#include <openssl/bio.h>
#include <openssl/rand.h>
#include <openssl/x509.h>
#include <openssl/evp.h>

#include <push/exception/push_exception.hpp>

namespace push {
namespace detail {
namespace p12 {

    static std::pair<std::string, std::string> extract_cert_key_pem(const std::string& p12_file, const std::string& p12_pass)
    {
        FILE *p12_f = fopen(p12_file.c_str(), "rb");
        
        if(!p12_f)
        {
            throw exception::push_exception("can't open file " + p12_file);
        }
        
        PKCS12* p12 = d2i_PKCS12_fp(p12_f, NULL);
        fclose(p12_f);
        
        if(!p12)
        {
            throw exception::push_exception("can't read p12 file, access rights?");
        }
        
        EVP_PKEY* pkey = NULL;
        X509* cert = NULL;
        STACK_OF(X509)* ca = NULL;
        
        if(!PKCS12_parse(
            p12,
            p12_pass.c_str(),
            &pkey,
            &cert,
            &ca)
           )
        {
            throw exception::push_exception(std::string(ERR_reason_error_string(ERR_get_error())));
        }
        
        PKCS12_free(p12);
        sk_X509_free(ca);
        
        BIO* b = BIO_new(BIO_s_mem());
        
        void* ps = NULL;
        if(!p12_pass.empty())
        {
            ps = (void*)p12_pass.c_str();
        }
           
        if(!PEM_write_bio_PrivateKey(b, pkey, NULL, NULL, 0, 0, ps))
        {
            // error writing
            BIO_free(b);
            EVP_PKEY_free(pkey);
            throw exception::push_exception("can't write private key PEM to bio");
        }
        
        BUF_MEM *bptr;
        BIO_get_mem_ptr(b, &bptr);
        
        std::string pk_str(bptr->data, bptr->length);
        
        BIO_free(b);
        EVP_PKEY_free(pkey);
        
        b = BIO_new(BIO_s_mem());
        std::string cert_str;
        
        if (!PEM_write_bio_X509(b, cert))
        {
            X509_free(cert);
            BIO_free(b);
            throw exception::push_exception("can't write cert PEM to bio");
        }
        
        X509_free(cert);
        
        BIO_get_mem_ptr(b, &bptr);
        std::string tmp = std::string(bptr->data, bptr->length);
        
        cert_str.append(tmp);
        
        BIO_free(b);
        
        return std::make_pair(cert_str, pk_str);
    }

}; // p12
}; // detail
}; // push

#endif // _PUSH_SERVICE_P12_HPP_