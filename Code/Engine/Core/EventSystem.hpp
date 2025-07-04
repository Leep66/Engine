#pragma once
#include <vector>
#include <map>
#include <algorithm>
#include <cctype>
#include <string>


class NamedStrings;

typedef NamedStrings EventArgs;
typedef bool (EventCallbackFunction)(EventArgs& args);
struct EventSystemConfig
{

};

struct EventSubscription
{
	EventSubscription(EventCallbackFunction* functionPtr)
		: m_functionPtr(functionPtr) {}

	EventCallbackFunction* m_functionPtr = nullptr;
};

struct cmpCaseInsensitive {
	bool operator()(const std::string& a, const std::string& b) const {
		std::string lowerCaseA = a;
		std::string lowerCaseB = b;
		std::transform(lowerCaseA.begin(), lowerCaseA.end(), lowerCaseA.begin(),
			[](unsigned char c) { return (char)std::tolower(c); });
		std::transform(lowerCaseB.begin(), lowerCaseB.end(), lowerCaseB.begin(),
			[](unsigned char c) { return (char)std::tolower(c); });

		return lowerCaseA < lowerCaseB;
	}
};

typedef std::vector<EventSubscription*>		SubscriptionList; // Note: a list of pointers

class EventSystem
{
public:
	EventSystem(EventSystemConfig const& config)
		: m_config(config) {}

	~EventSystem() {}
	void Startup() {}
	void Shutdown() {}
	void BeginFrame() {}
	void EndFrame() {}

	void SubscribeEventCallbackFunction(std::string const& eventName, EventCallbackFunction* func);
	void UnsubscribeEventCallbackFunction(std::string const& eventName, EventCallbackFunction* func);
	int	FireEvent(std::string const& eventName, EventArgs& args);
	int	FireEvent(std::string const& eventName); // Calls the above function with a temporary empty args

	std::vector<std::string> GetRegisteredEventNames() const;

private:
	EventSystemConfig							m_config;
	std::map<std::string, SubscriptionList, cmpCaseInsensitive> m_subscriptionListsByEventName;
};


//------------------------------------------------------------------------------------------------
// Standalone global-namespace helper functions; these forward to "the" event system, if it exists.
// These give our event system a fundamental "built-in" feel in our Engine, i.e. language-like.
// #ToDo: write these
void SubscribeEventCallbackFunction(std::string const& eventName, EventCallbackFunction* func);
void UnsubscribeEventCallbackFunction(std::string const& eventName, EventCallbackFunction* func);
int FireEvent(std::string const& eventName, EventArgs& args);
int FireEvent(std::string const& eventName); // Calls the above function with a temporary empty args