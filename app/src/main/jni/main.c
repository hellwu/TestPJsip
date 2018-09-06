#include <stdio.h>
#include <pjlib.h>
#include <pjlib-util.h>
#include <pjsip.h>
#include <pjsip-ua/sip_regc.h>
#include "main.h"

pjsip_endpoint *endpt;
pj_caching_pool cache_pool;
pj_str_t registrar_uri;
char *userId = "10009";
char *sDomain = "192.168.1.202";
char *userPwd = "123456";

/************************************************************************/
/* Registrar for testing */
static pj_bool_t regs_rx_request(pjsip_rx_data *rdata);

struct pjsip_module regc_mod = {
        NULL, NULL,                /* prev, next.		*/
        {"registrar", 9},            /* Name.			*/
        -1,                    /* Id			*/
        PJSIP_MOD_PRIORITY_APPLICATION,        /* Priority			*/
        NULL,                    /* load()			*/
        NULL,                    /* start()			*/
        NULL,                    /* stop()			*/
        NULL,                    /* unload()			*/
        &regs_rx_request,            /* on_rx_request()		*/
        NULL,                    /* on_rx_response()		*/
        NULL,                    /* on_tx_request.		*/
        NULL,                    /* on_tx_response()		*/
        NULL,                    /* on_tsx_state()		*/
};

static pj_bool_t regs_rx_request(pjsip_rx_data *rdata) {
    LOGI("regs_rx_request...........................\n");
    pjsip_msg *msg = rdata->msg_info.msg;
    pjsip_hdr hdr;
    int code;
    char *buffer;
    pjsip_msg_body body;
    pj_status_t status;

    //除去非register方法
    if (msg->line.req.method.id != PJSIP_REGISTER_METHOD)
        return PJ_FALSE;
    code = msg->line.status.code;

    buffer = (char *) malloc(500);
    msg->body->print_body(msg->body, buffer, 500);
    LOGI("sip register response code = %d, body = %s", code, buffer);
    return PJ_TRUE;
}

/* regc callback */
static void client_cb(struct pjsip_regc_cbparam *param) {
    pjsip_regc_info info;
    pj_status_t status;
    LOGI("client_cb......................!!!\n");
    status = pjsip_regc_get_info(param->regc, &info);
    if (status != PJ_SUCCESS) {
        LOGI("client_cb regc get info failure!!!\n");
        return;
    }
}


int init_pjsip() {
    pj_status_t rc;
    pj_sockaddr_in addr;
    pjsip_transport *udp;
    pj_uint16_t port;
    char registrar_uri_buf[80];

    if ((rc = pj_init()) != PJ_SUCCESS) {
        LOGI("pj_init failure!!\n");
        return rc;
    }
    LOGI("1\n");
    pj_caching_pool_init(&cache_pool, &pj_pool_factory_default_policy, PJSIP_TEST_MEM_SIZE);
    LOGI("2\n");
    rc = pjsip_endpt_create(&cache_pool.factory, "endpt", &endpt);
    LOGI("3\n");
    if (rc != PJ_SUCCESS) {
        LOGI("endpt create failure!!\n");
        pj_caching_pool_destroy(&cache_pool);
        return rc;
    }

    /* Start transaction layer module. */
    rc = pjsip_tsx_layer_init_module(endpt);
    if (rc != PJ_SUCCESS) {
        LOGI("Error initializing transaction module status = %d", rc);
        goto on_return;
    }

    /* Create loop transport. */
    rc = pjsip_loop_start(endpt, NULL);
    if (rc != PJ_SUCCESS) {
        LOGI("pjsip loop start!!\n");
        goto on_return;
    }

    pj_sockaddr_in_init(&addr, 0, 0);
    /* Acquire existing transport, if any */
    rc = pjsip_endpt_acquire_transport(endpt, PJSIP_TRANSPORT_UDP, &addr, sizeof(addr), NULL, &udp);
    if (rc == PJ_SUCCESS) {
        port = pj_sockaddr_get_port(&udp->local_addr);
        pjsip_transport_dec_ref(udp);
        udp = NULL;
    } else {
        rc = pjsip_udp_transport_start(endpt, NULL, NULL, 1, &udp);
        if (rc != PJ_SUCCESS) {
            LOGI("error creating UDP transport %d", rc);
            goto on_return;
        }

        port = pj_sockaddr_get_port(&udp->local_addr);
    }

    /* Register registrar module */
    rc = pjsip_endpt_register_module(endpt, &regc_mod);
    if (rc != PJ_SUCCESS) {
        LOGI("registe module failure:%d\n", rc);
        goto on_return;
    }


    pj_ansi_snprintf(registrar_uri_buf, sizeof(registrar_uri_buf),
                     "sip:192.168.1.202:%d", (int) port);
    registrar_uri = pj_str(registrar_uri_buf);
    return rc;

    on_return:
    pjsip_endpt_destroy(endpt);
    pj_caching_pool_destroy(&cache_pool);
    return rc;
}

int regc() {
    pjsip_regc *regc;
    char ct[100];
    char cor[32];
    LOGI("regc()\n");
    sprintf(cor, "<sip:%s@%s>", userId, sDomain);
    LOGI("cor = %s\n", cor);
    const pj_str_t aor = pj_str(cor);
    pjsip_tx_data *tdata;
    pj_status_t status;

    status = pjsip_regc_create(endpt, NULL, &client_cb, &regc);
    if (status != PJ_SUCCESS) {
        LOGI("regc create failure!!\n");
    }

    pj_str_t contact[1];
    sprintf(ct, "<sip:%s@%s:5060;transport=udp>", userId, sDomain);
    LOGI("ct = %s\n", ct);
    contact[0] = pj_str(ct);

    registrar_uri = pj_str("sip:192.168.1.202");
    status = pjsip_regc_init(regc, &registrar_uri, &aor, &aor, 1, contact, 600);
    if (status != PJ_SUCCESS) {
        LOGI("pjsip_regc_init failure");
        pjsip_regc_destroy(regc);
    }

    pjsip_cred_info cred;

    pj_bzero(&cred, sizeof(cred));
    cred.realm = pj_str("*");
    cred.scheme = pj_str("digest");
    cred.username = pj_str(userId);
    cred.data_type = PJSIP_CRED_DATA_PLAIN_PASSWD;
    cred.data = pj_str(userPwd);

    status = pjsip_regc_set_credentials(regc, 1, &cred);

    if (status != PJ_SUCCESS) {
        pjsip_regc_destroy(regc);
        return status;
    }

    /* Register */
    status = pjsip_regc_register(regc, PJ_TRUE, &tdata);
    if (status != PJ_SUCCESS) {
        pjsip_regc_destroy(regc);
        return status;
    }

    status = pjsip_regc_send(regc, tdata);
    if (status != PJ_SUCCESS) {
        LOGI("pjsip_regc_send failure! status = %d\n", status);
    }
    LOGI("end regc!\n");
}

/* Log callback */
static void log_writer(int level, const char *buffer, int len) {
    __android_log_write(ANDROID_LOG_INFO, "pjsua", buffer);
}

int setlog() {
    /* Redirect log function to ours */
    pj_log_set_log_func(&log_writer);
    /* Set log level */
    pj_log_set_level(4);
}

void test() {
    LOGI("TestSIP!\n");
    setlog();
    //
    init_pjsip();
    regc();
}

//int main(){
//	LOGI("TestSIP!\n");
//	setlog();
//	//
//	init_pjsip();
//	regc();
//	return 0;
//}
