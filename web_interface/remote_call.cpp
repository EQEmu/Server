#include "web_interface.h"
#include "remote_call.h"

void WriteWebCallResponseString(per_session_data_eqemu *session, rapidjson::Document &doc, std::string result, bool error, bool send_no_id) {
	if (doc.HasMember("id") || send_no_id) {
		rapidjson::StringBuffer s;
		rapidjson::Writer<rapidjson::StringBuffer> writer(s);
		writer.StartObject();
		writer.String("id");
		if (send_no_id) {
			writer.Null();
		}
		else {
			writer.String(doc["id"].GetString());
		}

		writer.String("result");
		writer.StartObject();
		writer.String("value");
		writer.String(result.c_str());
		writer.EndObject();

		writer.String("error");
		if (error) {
			writer.Bool(true);
		}
		else {
			writer.Null();
		}
		writer.EndObject();
		session->send_queue->push_back(s.GetString());
	}
}

void WriteWebCallResponseInt(per_session_data_eqemu *session, rapidjson::Document &doc, int result, bool error, bool send_no_id) {
	if (doc.HasMember("id") || send_no_id) {
		rapidjson::StringBuffer s;
		rapidjson::Writer<rapidjson::StringBuffer> writer(s);
		writer.StartObject();
		writer.String("id");
		if (send_no_id) {
			writer.Null();
		}
		else {
			writer.String(doc["id"].GetString());
		}

		writer.String("result");
		writer.StartObject();
		writer.String("value");
		writer.Int(result);
		writer.EndObject();

		writer.String("error");
		if (error) {
			writer.Bool(true);
		}
		else {
			writer.Null();
		}
		writer.EndObject();
		session->send_queue->push_back(s.GetString());
	}
}

void WriteWebCallResponseBoolean(per_session_data_eqemu *session, rapidjson::Document &doc, bool result, bool error, bool send_no_id) {
	if (doc.HasMember("id") || send_no_id) {
		rapidjson::StringBuffer s;
		rapidjson::Writer<rapidjson::StringBuffer> writer(s);
		writer.StartObject();
		writer.String("id");
		if (send_no_id) {
			writer.Null();
		}
		else {
			writer.String(doc["id"].GetString());
		}

		writer.String("result");
		writer.StartObject();
		writer.String("value");
		writer.Bool(result);
		writer.EndObject();

		writer.String("error");
		if (error) {
			writer.Bool(true);
		}
		else {
			writer.Null();
		}
		writer.EndObject();
		session->send_queue->push_back(s.GetString());
	}
}

int CheckTokenAuthorization(per_session_data_eqemu *session) {
	//todo: actually check this against a table of tokens that is updated periodically
	//right now i have just one entry harded coded for testing purposes
	if (session->auth.compare("c5b80ec8-4174-4c4c-d332-dbf3c3a551fc") == 0) {
		return 255;
	}

	return 0;
}
