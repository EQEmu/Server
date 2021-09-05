#include "dialogue_window.h"

void DialogueWindow::Render(Client *c, std::string markdown)
{
	std::string output = markdown;

	// this is the NPC that the client is interacting with if there is dialogue going on
	Mob *target;
	if (c->GetTarget()) {
		target = c->GetTarget();
	}

	// zero this out
	c->SetEntityVariable(DIAWIND_RESPONSE_KEY.c_str(), "");

	// simple find and replace for the markdown
	find_replace(output, "~", "</c>");
	find_replace(output, "{y}", "<c \"#CCFF33\">");
	find_replace(output, "{lb}", "<c \"#00FFFF\">");
	find_replace(output, "{r}", "<c \"#FF0000\">");
	find_replace(output, "{g}", "<c \"#00FF00\">");
	find_replace(output, "{gold}", "<c \"#FFFF66\">");
	find_replace(output, "{orange}", "<c \"#FFA500\">");
	find_replace(output, "{gray}", "<c \"#808080\">");
	find_replace(output, "{tan}", "<c \"#daa520\">");
	find_replace(output, "{bullet}", "•");
	find_replace(output, "{name}", "$name");
	find_replace(output, "{linebreak}", "--------------------------------------------------------------------");
	find_replace(output, "{rowpad}", R"(<tr><td>{tdpad}<"td><td>{tdpad}<"td><"tr>)");
	find_replace(output, "{tdpad}", "----------------------");
	find_replace(output, "{in}", "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;");

	// mysterious voice
	bool render_mysterious_voice = false;
	if (markdown.find("mysterious") != std::string::npos) {
		render_mysterious_voice = true;
		LogDiaWind("Client [{}] Rendering mysterious voice", c->GetCleanName());
		find_replace(output, "mysterious", "");
	}

	// noquotes
	bool render_noquotes = false;
	if (markdown.find("noquotes") != std::string::npos) {
		render_noquotes = true;
		LogDiaWind("Client [{}] Rendering noquotes", c->GetCleanName());
		find_replace(output, "noquotes", "");
	}

	// nobracket
	bool render_nobracket = false;
	if (markdown.find("nobracket") != std::string::npos) {
		render_nobracket = true;
		LogDiaWind("Client [{}] Rendering nobracket", c->GetCleanName());
		find_replace(output, "nobracket", "");
	}

	// animations
	std::string animation = get_between(output, "+", "+");
	if (!animation.empty()) {
		LogDiaWind("Client [{}] Animation is not empty, contents are [{}]", c->GetCleanName(), animation);
		find_replace(output, fmt::format("+{}+", animation), "");

		// we treat the animation field differently if it is a number
		if (StringIsNumber(animation)) {
			LogDiaWindDetail("Client [{}] Animation is a number, firing animation [{}]", c->GetCleanName(), animation);
			target->DoAnim(std::stoi(animation));
		}
		else {
			for (auto &a: animations) {
				if (a.first.find(str_tolower(animation)) != std::string::npos) {
					LogDiaWindDetail(
						"Client [{}] Animation is a string, firing animation [{}] [{}]",
						c->GetCleanName(),
						a.second,
						a.first
					);
					target->DoAnim(a.second);
				}
			}
		}
	}

	// window expire time
	std::string expire_time           = get_between(output, "=", "=");
	uint32      window_expire_seconds = 0;
	if (!expire_time.empty()) {
		LogDiaWind("Client [{}] Window expire time is not empty, contents are [{}]", c->GetCleanName(), expire_time);
		find_replace(output, fmt::format("={}=", expire_time), "");

		// we treat the animation field differently if it is a number
		if (StringIsNumber(expire_time)) {
			LogDiaWindDetail(
				"Client [{}] Window expire time is a number, setting expiration to [{}]",
				c->GetCleanName(),
				expire_time
			);
			window_expire_seconds = std::stoi(expire_time);
		}
	}

	uint32 popup_id       = POPUPID_DIAWIND;
	uint32 negative_id    = 0;
	char   *button_name_0 = nullptr;
	char   *button_name_1 = nullptr;
	uint32 sound_controls = 0;

	// window type
	std::string wintype;
	if (markdown.find("wintype") != std::string::npos) {
		LogDiaWind("Client [{}] Rendering wintype option", c->GetCleanName());

		auto first_split = split_string(output, "wintype:");
		if (!first_split.empty()) {

			// assumed that there is more after the wintype declaration
			// wintype:0 +animation+ etc.
			auto second_split = split_string(first_split[1], " ");
			if (!second_split.empty()) {
				wintype = second_split[0];
				LogDiaWindDetail("Client [{}] Rendering wintype option wintype [{}]", c->GetCleanName(), wintype);
			}

			// if we're dealing with a string that is at the end
			// example wintype:0");
			if (first_split[1].length() == 1) {
				wintype = first_split[1];
				LogDiaWindDetail(
					"Client [{}] Rendering wintype (end) option wintype [{}]",
					c->GetCleanName(),
					wintype
				);
			}

			find_replace(output, fmt::format("wintype:{}", wintype), "");
		}
	}

	// popupid
	std::string popupid;
	if (markdown.find("popupid") != std::string::npos) {
		LogDiaWind("Client [{}] Rendering popupid option", c->GetCleanName());

		auto first_split = split_string(output, "popupid:");
		if (!first_split.empty()) {

			// assumed that there is more after the popupid declaration
			// popupid:0 +animation+ etc.
			auto second_split = split_string(first_split[1], " ");
			if (!second_split.empty()) {
				popupid = second_split[0];
				LogDiaWindDetail("Client [{}] Rendering popupid option popupid [{}]", c->GetCleanName(), popupid);
			}

			// if we're dealing with a string that is at the end
			// example popupid:0");
			if (first_split[1].length() == 1) {
				popupid = first_split[1];
				LogDiaWindDetail(
					"Client [{}] Rendering popupid (end) option popupid [{}]",
					c->GetCleanName(),
					popupid
				);
			}

			find_replace(output, fmt::format("popupid:{}", popupid), "");

			// set the popup id
			if (!popupid.empty()) {
				popup_id = (StringIsNumber(popupid) ? std::atoi(popupid.c_str()) : 0);
			}
		}
	}

	// bracket responses
	std::vector<std::string> responses;
	std::vector<std::string> bracket_responses;
	if (markdown.find('[') != std::string::npos && markdown.find(']') != std::string::npos) {
		// copy
		std::string content = output;

		// while brackets still exist
		int response_index = 0;
		while (content.find('[') != std::string::npos && content.find(']') != std::string::npos) {
			std::string bracket_message = get_between(content, "[", "]");

			LogDiaWindDetail(
				"Client [{}] Rendering responses ({}) [{}]",
				c->GetCleanName(),
				response_index,
				bracket_message
			);

			// pop message onto responses
			responses.emplace_back(bracket_message);

			// pop the response off of the message
			find_replace(content, fmt::format("[{}]", bracket_message), "");

			response_index++;
		}
	}

	// build saylinks
	if (responses.size() > 1) {
		for (auto &r: responses) {
			bracket_responses.emplace_back(
				fmt::format("[{}]", EQ::SayLinkEngine::GenerateQuestSaylink(r, false, r))
			);
		}
	}

	// handle silent prompts from the [> silent syntax
	std::string silent_message;
	if (responses.empty() && markdown.find('[') != std::string::npos && markdown.find('>') != std::string::npos) {
		silent_message = get_between(output, "[", ">");

		// temporary and used during the response
		c->SetEntityVariable(DIAWIND_RESPONSE_KEY.c_str(), silent_message.c_str());

		// pop the silent message off
		find_replace(output, fmt::format("[{}>", silent_message), "");
	}
	else if (!responses.empty()) {
		// handle silent prompts from the single respond bracket syntax []
		silent_message = responses[0];

		// temporary and used during the response
		c->SetEntityVariable(DIAWIND_RESPONSE_KEY.c_str(), silent_message.c_str());

		// pop the silent message off
		find_replace(output, fmt::format("[{}]", silent_message), "");
	}

	// strip brackets
	if (render_nobracket) {
		find_replace(output, "[", "");
		find_replace(output, "]", "");
	}

	// render title
	std::string title;
	std::string speaking;
	if (target) {
		speaking = fmt::format("{} says", target->GetCleanName());
	}

	if (render_mysterious_voice) {
		speaking = "A Mysterious Voice says";
	}

	title = fmt::format("Dialogue [{}]", speaking);

	// render quotes
	std::string quote_string = "'";
	if (render_noquotes) {
		quote_string = "";
	}

	// click response
	// window type response
	uint32      window_type           = (StringIsNumber(wintype) ? std::atoi(wintype.c_str()) : 0);
	std::string click_response_button = (window_type == 1 ? "Yes" : "OK");
	std::string click_response        = fmt::format(
		"<c \"#F07F00\">Click [{}] to continue...</c>",
		click_response_button
	);

	// different response when a timer is set
	if (window_expire_seconds > 0) {
		click_response = fmt::format(
			"<c \"#F07F00\">This message will disappear in {} second(s)...</c>",
			window_expire_seconds
		);
	}

	// respond with silent message
	if (!silent_message.empty()) {
		click_response = fmt::format(
			"<c \"#F07F00\">Click [{}] to respond with [{}]...</c>",
			click_response_button,
			silent_message
		);
	}

	// post processing of color markdowns
	// {spring_green_1} = <c "#5EFB6E">
	if (markdown.find('{') != std::string::npos && markdown.find('}') != std::string::npos) {

		// while brackets still exist
		int tag_index = 0;
		while (output.find('{') != std::string::npos && output.find('}') != std::string::npos) {
			std::string color_tag = get_between(output, "{", "}");

			LogDiaWindDetail(
				"Client [{}] Rendering color tags ({}) [{}]",
				c->GetCleanName(),
				tag_index,
				color_tag
			);

			std::string html_tag;
			for (const auto& color : html_colors) {
				if (color_tag.find(color.first) != std::string::npos) {
					// build html tag
					html_tag = fmt::format("<c \"{}\">", color.second);
					// pop the response off of the message
					find_replace(output, fmt::format("{{{}}}", color.first), html_tag);
				}
			}

			tag_index++;
		}
	}


	// build the final output string
	std::string final_output;
	final_output = fmt::format("{}{}{} <br><br> {}", quote_string, output, quote_string, click_response);

	// send popup
	c->SendFullPopup(
		title.c_str(),
		final_output.c_str(),
		popup_id,
		negative_id,
		window_type,
		window_expire_seconds,
		button_name_0,
		button_name_1,
		sound_controls
	);

	// if multiple brackets are presented, send message
	if (!bracket_responses.empty()) {
		c->Message(Chat::White, " --- Select Response from Options --- ");
		c->Message(Chat::White, implode(" ", bracket_responses).c_str());
	}
}
