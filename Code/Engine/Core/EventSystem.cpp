#include "Engine/Core/EventSystem.hpp"
#include "Engine/Input/NamedStrings.hpp"


extern EventSystem* g_theEventSystem;

void EventSystem::SubscribeEventCallbackFunction(std::string const& eventName, EventCallbackFunction* func)
{
	SubscriptionList& subscribersForThisEvent = m_subscriptionListsByEventName[eventName];
	EventSubscription* newEventSubscription = new EventSubscription(func);
	subscribersForThisEvent.push_back(newEventSubscription); // #ToDo: check for null entries to fill in first
}


//------------------------------------------------------------------------------------------------
void EventSystem::UnsubscribeEventCallbackFunction(std::string const& eventName, EventCallbackFunction* func)
{
	std::map< std::string, SubscriptionList >::iterator found = m_subscriptionListsByEventName.find(eventName);
	if (found == m_subscriptionListsByEventName.end())
	{
		return; // Nobody subscribed to this event
	}

	SubscriptionList& subscribersForThisEvent = found->second;
	int numSubscribers = static_cast<int>(subscribersForThisEvent.size());
	for (int i = 0; i < numSubscribers; ++i)
	{
		EventSubscription*& subscriber = subscribersForThisEvent[i];
		if (subscriber && subscriber->m_functionPtr == func)
		{
			subscriber = nullptr;
		}
	}
}


//------------------------------------------------------------------------------------------------
int EventSystem::FireEvent(std::string const& eventName, EventArgs& args)
{
	printf("  Firing event \"%s\"...\n", eventName.c_str());

	std::map< std::string, SubscriptionList >::iterator found = m_subscriptionListsByEventName.find(eventName);
	if (found == m_subscriptionListsByEventName.end())
	{
		return 0; // Nobody subscribed to this event
	}

	// Found a list of subscribers for this event; call each one in turn (or until someone "consumes" the event)
	SubscriptionList& subscribersForThisEvent = found->second;
	int numSubscribers = static_cast<int>(subscribersForThisEvent.size());
	for (int i = 0; i < numSubscribers; ++i)
	{
		EventSubscription* subscriber = subscribersForThisEvent[i];
		if (subscriber)
		{
			bool wasConsumed = subscriber->m_functionPtr(args); // Execute the subscriber's callback function!
			if (wasConsumed)
			{
				break; // Event was "consumed" by this subscriber; stop notifying any other subscribers!
			}
		}
	}

	return numSubscribers;
}


//------------------------------------------------------------------------------------------------
int EventSystem::FireEvent(std::string const& eventName)
{
	EventArgs emptyArgs;
	return FireEvent(eventName, emptyArgs);
}

std::vector<std::string> EventSystem::GetRegisteredEventNames() const
{
	std::vector<std::string> eventNames;
	for (const auto& pair : m_subscriptionListsByEventName) {
		eventNames.push_back(pair.first);
	}
	return eventNames;
}

void SubscribeEventCallbackFunction(std::string const& eventName, EventCallbackFunction* func)
{
	if (g_theEventSystem != nullptr)
	{
		g_theEventSystem->SubscribeEventCallbackFunction(eventName, func);
	}
}

void UnsubscribeEventCallbackFunction(std::string const& eventName, EventCallbackFunction* func)
{
	if (g_theEventSystem != nullptr)
	{
		g_theEventSystem->UnsubscribeEventCallbackFunction(eventName, func);
	}
}

int FireEvent(std::string const& eventName, EventArgs& args)
{
	if (g_theEventSystem != nullptr)
	{
		return g_theEventSystem->FireEvent(eventName, args);
	}
	return 0;

}

int FireEvent(std::string const& eventName)
{
	if (g_theEventSystem != nullptr)
	{
		return g_theEventSystem->FireEvent(eventName);
	}
	return 0;
}






