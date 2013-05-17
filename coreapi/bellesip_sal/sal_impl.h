/*
linphone
Copyright (C) 2012  Belledonne Communications, Grenoble, France

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef SAL_IMPL_H_
#define SAL_IMPL_H_

#include "sal/sal.h"
#include "belle-sip/belle-sip.h"
#include "belle-sip/belle-sdp.h"



struct Sal{
	SalCallbacks callbacks;
	MSList *pending_auths;/*MSList of SalOp */
	belle_sip_stack_t* stack;
	belle_sip_provider_t *prov;
	belle_sip_header_user_agent_t* user_agent;
	belle_sip_listener_t *listener;
	void *up; /*user pointer*/
	int session_expires;
	unsigned int keep_alive;
	char *root_ca;
	char *uuid;
	bool_t one_matching_codec;
	bool_t use_tcp_tls_keep_alive;
	bool_t nat_helper_enabled;
	bool_t tls_verify;
	bool_t tls_verify_cn;
	bool_t use_dates;
};

typedef enum SalOpState {
	SalOpStateEarly=0
	,SalOpStateActive
	,SalOpStateTerminating /*this state is used to wait until a proceeding state, so we can send the cancel*/
	,SalOpStateTerminated
}SalOpState;

const char* sal_op_state_to_string(SalOpState value);

typedef enum SalOpDir {
	SalOpDirIncoming=0
	,SalOpDirOutgoing
}SalOpDir_t;
typedef enum SalOpType {
	SalOpUnknown,
	SalOpRegister,
	SalOpCall,
	SalOpMessage,
	SalOpPresence,
	SalOpPublish,
	SalOpInfo
}SalOpType_t;
const char* sal_op_type_to_string(const SalOpType_t type);

struct SalOp{
	SalOpBase base;
	belle_sip_listener_callbacks_t callbacks;
	belle_sip_client_transaction_t *pending_auth_transaction;
	belle_sip_server_transaction_t* pending_server_trans;
	belle_sip_client_transaction_t* pending_client_trans;
	SalAuthInfo* auth_info;
	bool_t sdp_offering;
	belle_sip_dialog_t* dialog;
	belle_sip_header_replaces_t *replaces;
	belle_sip_header_referred_by_t *referred_by;
	bool_t auto_answer_asked;
	SalMediaDescription *result;
	belle_sdp_session_description_t *sdp_answer;
	bool_t supports_session_timers;
	SalOpState state;
	SalOpDir_t dir;
	belle_sip_refresher_t* refresher;
	int ref;
	SalOpType_t type;
};


belle_sdp_session_description_t * media_description_to_sdp(const SalMediaDescription *sal);
int sdp_to_media_description(belle_sdp_session_description_t  *sdp, SalMediaDescription *desc);
belle_sip_request_t* sal_op_build_request(SalOp *op,const char* method);


void sal_op_call_fill_cbs(SalOp*op);
void set_or_update_dialog(SalOp* op, belle_sip_dialog_t* dialog);

/*return reffed op*/
SalOp* sal_op_ref(SalOp* op);
/*return null, destroy op if ref count =0*/
void* sal_op_unref(SalOp* op);
void sal_op_release_impl(SalOp *op);

void sal_op_set_remote_ua(SalOp*op,belle_sip_message_t* message);
int sal_op_send_request(SalOp* op, belle_sip_request_t* request);
int sal_op_send_request_with_expires(SalOp* op, belle_sip_request_t* request,int expires);
void sal_op_resend_request(SalOp* op, belle_sip_request_t* request);
int sal_op_send_and_create_refresher(SalOp* op,belle_sip_request_t* req, int expires,belle_sip_refresher_listener_t listener );
belle_sip_response_t *sal_op_create_response_from_request(SalOp *op, belle_sip_request_t *req, int code);

void sal_process_authentication(SalOp *op);
belle_sip_header_contact_t* sal_op_create_contact(SalOp *op,belle_sip_header_from_t* from_header) ;

bool_t sal_compute_sal_errors(belle_sip_response_t* response,SalError* sal_err,SalReason* sal_reason,char* reason, size_t reason_size);
void sal_compute_sal_errors_from_code(int code ,SalError* sal_err,SalReason* sal_reason) ;
/*presence*/
void sal_op_presence_fill_cbs(SalOp*op);
/*messaging*/
void sal_op_message_fill_cbs(SalOp*op);

/*info*/
void sal_op_info_fill_cbs(SalOp*op);

/*call transfer*/
void sal_op_process_refer(SalOp *op, const belle_sip_request_event_t *event);
void sal_op_call_process_notify(SalOp *op, const belle_sip_request_event_t *event);
/*create SalAuthInfo by copying username and realm from suth event*/
SalAuthInfo* sal_auth_info_create(belle_sip_auth_event_t* event) ;
void sal_add_pending_auth(Sal *sal, SalOp *op);
void sal_remove_pending_auth(Sal *sal, SalOp *op);
void sal_add_presence_info(belle_sip_message_t *notify, SalPresenceStatus online_status);

belle_sip_response_t *sal_create_response_from_request(Sal *sal, belle_sip_request_t *req, int code);

void sal_op_assign_recv_headers(SalOp *op, belle_sip_message_t *incoming);

#endif /* SAL_IMPL_H_ */
