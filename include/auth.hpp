/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <string>

// Modern std::string overload
std::string createAuthHeader(const std::string& user,
                           const std::string& password,
                           const char* method,
                           const std::string& uri,
                           const char* msgbody,
                           const char* auth,
                           const std::string& aka_OP = {},
                           const std::string& aka_AMF = {},
                           const std::string& aka_K = {},
                           unsigned int nonce_count = 1);

int verifyAuthHeader(const char *user, const char *password,
                     const char *method, const char *auth,
                     const char *msgbody);
int getAuthParameter(const char *name, const char *header, char *result,
                     int len);
