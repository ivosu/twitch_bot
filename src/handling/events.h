#ifndef EVENTS_H
#define EVENTS_H

#include <string>
#include <variant>

class incoming_whisper {
  public:
	const std::string& from() const {
		return m_from;
	}

	const std::string& text() const {
		return m_text;
	}

  private:
	std::string m_from;
	std::string m_text;
};

class outgoing_whisper {
  public:
  private:
	std::string m_to;
	std::string m_text;
};

class incoming_message {
  public:
	incoming_message(std::string channel, std::string from, std::string text)
		: m_channel(std::move(channel)),
		  m_from(std::move(from)),
		  m_text(std::move(text))
	{}

	const std::string& channel() const {
		return m_channel;
	}

	const std::string& from() const {
		return m_from;
	}

	const std::string& text() const {
		return m_text;
	}
  private:
	std::string m_channel;
	std::string m_from;
	std::string m_text;
};

class outgoing_message {
  public:
	outgoing_message(std::string channel, std::string text)
	  : m_channel(std::move(channel)),
		m_text(std::move(text))
	{}

	const std::string& channel() const {
		return m_channel;
	}

	const std::string& text() const {
		return m_text;
	}
  private:
	std::string m_channel;
	std::string m_text;
};

// TODO subscriptions, room state change

using incoming_event = std::variant<incoming_message, incoming_whisper>;

using outgoing_event = std::variant<outgoing_whisper, outgoing_message, void>;

#endif //EVENTS_H
