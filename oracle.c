#include <stdio.h>
#include <stdlib.h>
#include <oci.h>

// Error handling macro
#define CHECK_OCI(status, handle, type, msg) \
    if ((status) != OCI_SUCCESS) { \
        text errbuf[512]; sb4 errcode = 0; \
        OCIErrorGet(handle, 1, NULL, &errcode, errbuf, sizeof(errbuf), type); \
        fprintf(stderr, "Error: %s\n%s\n", msg, errbuf); \
        exit(EXIT_FAILURE); \
    }

int main() {
    OCIEnv     *envhp  = NULL;
    OCIError   *errhp  = NULL;
    OCIServer  *srvhp  = NULL;
    OCISvcCtx  *svchp  = NULL;
    OCISession *authp  = NULL;
    sword status;

    const char *username = "jjcorr-user";
    const char *password = "pasword12345"
    const char *conn_str = "your_oracle_host:1521/your_service_name";

    

    // Initialize OCI environment
    status = OCIEnvCreate(&envhp, OCI_DEFAULT, NULL, NULL, NULL, NULL, 0, NULL);
    CHECK_OCI(status, errhp, OCI_HTYPE_ERROR, "Failed to create environment");

    OCIHandleAlloc(envhp, (void**)&errhp, OCI_HTYPE_ERROR, 0, NULL);
    OCIHandleAlloc(envhp, (void**)&srvhp, OCI_HTYPE_SERVER, 0, NULL);
    OCIHandleAlloc(envhp, (void**)&svchp, OCI_HTYPE_SVCCTX, 0, NULL);

    // Attach to server
    status = OCIServerAttach(srvhp, errhp, (text*)conn_str, strlen(conn_str), OCI_DEFAULT);
    CHECK_OCI(status, errhp, OCI_HTYPE_ERROR, "Failed to attach to server");

    // Set server handle attribute in service context
    OCIAttrSet(svchp, OCI_HTYPE_SVCCTX, srvhp, 0, OCI_ATTR_SERVER, errhp);

    // Allocate and set up session
    OCIHandleAlloc(envhp, (void**)&authp, OCI_HTYPE_SESSION, 0, NULL);
    OCIAttrSet(authp, OCI_HTYPE_SESSION, (void*)username, strlen(username), OCI_ATTR_USERNAME, errhp);
    OCIAttrSet(authp, OCI_HTYPE_SESSION, (void*)password, strlen(password), OCI_ATTR_PASSWORD, errhp);

    // Begin session
    status = OCISessionBegin(svchp, errhp, authp, OCI_CRED_RDBMS, OCI_DEFAULT);
    CHECK_OCI(status, errhp, OCI_HTYPE_ERROR, "Failed to start session");

    // Set session in service context
    OCIAttrSet(svchp, OCI_HTYPE_SVCCTX, authp, 0, OCI_ATTR_SESSION, errhp);

    printf("Successfully connected to Oracle database as %s.\n", username);

    // End session and clean up
    OCISessionEnd(svchp, errhp, authp, OCI_DEFAULT);
    OCIServerDetach(srvhp, errhp, OCI_DEFAULT);

    OCIHandleFree(authp, OCI_HTYPE_SESSION);
    OCIHandleFree(svchp, OCI_HTYPE_SVCCTX);
    OCIHandleFree(srvhp, OCI_HTYPE_SERVER);
    OCIHandleFree(errhp, OCI_HTYPE_ERROR);
    OCIHandleFree(envhp, OCI_HTYPE_ENV);

    return EXIT_SUCCESS;
}
