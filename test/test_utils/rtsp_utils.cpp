#include "httpauth.h"
#include <cstring>
#include <glog/logging.h>
#include <gtest/gtest.h>
#include <iostream>

using namespace std;

TEST(TEST_UTILS, DIGEST_AUTH)
{
    HTTPAuthState *state = new HTTPAuthState;
    memset(state, 0, sizeof(HTTPAuthState));
    state->auth_type = HTTP_AUTH_DIGEST;
    sprintf(state->digest_params.nonce, "f610b28c7953f435cd7f80014c24bbac");
    sprintf(state->realm, "Login to aa453f50378c821b6b22123cda43cba4");

    std::string response =
        make_digest_auth(state, "admin", "password", "rtsp://192.168.1.49:554/cam/realmonitor?channel=1&subtype=0", "OPTIONS");
    std::string correctResponse = "4b99786b2a560d61f23d38898418349d";
    EXPECT_STREQ(response.c_str(), correctResponse.c_str());
}

int main(int argc, char *argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
