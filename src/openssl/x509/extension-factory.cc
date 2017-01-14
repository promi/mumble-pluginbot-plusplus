/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
    mumble-pluginbot-plusplus - An extensible Mumble bot
    Copyright (c) 2016 Phobos (promi) <prometheus@unterderbruecke.de>
    Copyright (c) 2017 Phobos (promi) <prometheus@unterderbruecke.de>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "openssl/x509/extension-factory.hh"

#include <openssl/x509.h>
#include <openssl/x509v3.h>

namespace OpenSSL::X509
{
  ExtensionFactory::ExtensionFactory (const OpenSSL::X509::Certificate
                                      &issuer_certificate,
                                      const OpenSSL::X509::Certificate &subject_certificate)
    : m_issuer_certificate (issuer_certificate),
      m_subject_certificate (subject_certificate)
  {
  }

  Extension ExtensionFactory::create_extension (const std::string &oid,
      const std::string &value,
      bool critical)
  {
    // Is name a long name?
    int nid = OBJ_ln2nid (oid.c_str ());
    if (nid == 0)
      {
        // Is it a short name?
        nid = OBJ_sn2nid (oid.c_str ());
        if (nid == 0)
          {
            throw std::runtime_error ("unknown OID: " + oid);
          }
      }
    std::string combined_value;
    if (critical)
      {
        combined_value = "critical,";
      }
    combined_value += value;
    X509_EXTENSION *ext = nullptr;
    X509V3_CTX ctx;
    X509V3_set_ctx_nodb (&ctx);
    X509V3_set_ctx (&ctx, const_cast<::X509*> (m_issuer_certificate.data ()),
                    const_cast<::X509*> (m_subject_certificate.data ()), nullptr, nullptr,
                    0);
    ext = X509V3_EXT_conf_nid (nullptr, &ctx, nid,
                               const_cast<char*> (combined_value.c_str ()));
    if (ext == nullptr)
      {
        throw std::runtime_error ("X509V3_EXT_conf_nid () failed");
      }
    return Extension (ext);
  }
}
