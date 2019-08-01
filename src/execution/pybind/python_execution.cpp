//
// Created by strejivo on 7/16/19.
//

#include "python_execution.h"
#include "../../twitch_bot.h"
#include <pybind11/embed.h>
#include <pybind11/stl.h>

PYBIND11_EMBEDDED_MODULE(pybind_irc, m) {
	pybind11::class_<irc::prefix_t>(m, "irc_prefix")
			.def(pybind11::init<
					const std::string&,
					const std::optional<std::string>&,
					const std::optional<std::string>&
			>())
			.def("main", &irc::prefix_t::main)
			.def("host", &irc::prefix_t::host)
			.def("user", &irc::prefix_t::user);

	pybind11::class_<irc::message>(m, "irc_message")
			.def(pybind11::init<
					const irc::tags_t&,
					const std::optional<irc::prefix_t>&,
					const std::string&,
					const std::vector<std::string>&
			>())
			.def("prefix", &irc::message::prefix)
			.def("command", &irc::message::command)
			.def("tags", &irc::message::tags)
			.def("params", &irc::message::params);

	pybind11::class_<twitch_bot>(m, "twitch_bot")
			.def("send_message", &twitch_bot::send_message)
			.def_static("get_user_name_from_user_notice_tags", &twitch_bot::get_user_name_from_user_notice_tags)
			.def_static("get_user_name_private_message", &twitch_bot::get_user_name_private_message)
			.def_static("get_gifted_recipient_user_name", &twitch_bot::get_gifted_recipient_user_name);
}

void
python_execution::executeOnMessage(const std::string& channel, const std::string& code, const irc::message& message,
								   const twitch_bot& bot,
								   const db_message_communicator& comm) {
	pybind11::scoped_interpreter guard{};

	auto globals = pybind11::globals();
	auto message_module = pybind11::module::import("pybind_irc");

	std::FILE* tmpfile = std::tmpfile();
	//pybind11::module::impo
	auto locals = pybind11::dict();
	try {
		pybind11::exec(code, globals, locals);
	}
	catch (const pybind11::builtin_exception& e) {}

	if (!locals.contains("onMessage")) {
		throw "asd";
	}
	try {
		locals["onMessage"](message, bot, comm);
	} catch (...) {}
}
