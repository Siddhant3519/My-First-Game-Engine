#include "EventSystem.hpp"

//--------------------------------------------------------------------------------------------------
EventSystem* g_theEventSystem = nullptr;


//--------------------------------------------------------------------------------------------------
void EventSystem::SubscribeEventCallbackFunction(std::string const& eventName, EventCallbackFunction functionPtr)
{
	m_subscriptionListMutex.lock();

	SubscriptionList& subscribersForThisEvent = m_subscriptionListByEventName[eventName];
	subscribersForThisEvent.push_back(functionPtr);

	m_subscriptionListMutex.unlock();
}


//--------------------------------------------------------------------------------------------------
void EventSystem::UnsubscribeEventCallbackFunction(std::string const& eventName, EventCallbackFunction functionPtr)
{
	m_subscriptionListMutex.lock();

	SubscriptionList& subscribersForThisEvent = m_subscriptionListByEventName[eventName];
	for (int subscriberIndex = 0; subscriberIndex < (int)subscribersForThisEvent.size(); ++subscriberIndex)
	{
		if (subscribersForThisEvent[subscriberIndex] == functionPtr)
		{
			subscribersForThisEvent[subscriberIndex] = nullptr;
		}
	}

	m_subscriptionListMutex.unlock();
}


//--------------------------------------------------------------------------------------------------
void EventSystem::UnsubscribeFromAllEvents(EventCallbackFunction functionPtr)
{
	std::map<std::string, SubscriptionList>::iterator eventIter;
	
	m_subscriptionListMutex.lock();
	std::map<std::string, SubscriptionList>::iterator beginIter = m_subscriptionListByEventName.begin();
	std::map<std::string, SubscriptionList>::iterator endIter	= m_subscriptionListByEventName.end();
	m_subscriptionListMutex.unlock();

	for (eventIter = beginIter; eventIter != endIter; ++eventIter)
	{
		std::string const& eventName = eventIter->first;
		UnsubscribeEventCallbackFunction(eventName, functionPtr);
	}
}


//--------------------------------------------------------------------------------------------------
void EventSystem::FireEvent(std::string const& eventName, EventArgs& args)
{
	m_subscriptionListMutex.lock();
	SubscriptionList& subscribersForThisEvent = m_subscriptionListByEventName[eventName];
	for (int subscriberIndex = 0; subscriberIndex < (int)subscribersForThisEvent.size(); ++subscriberIndex)
	{
		EventCallbackFunction callbackFunctionPtr = subscribersForThisEvent[subscriberIndex];
		if (callbackFunctionPtr != nullptr)
		{
			m_subscriptionListMutex.unlock();
			bool wasConsumed = callbackFunctionPtr(args);
			m_subscriptionListMutex.lock();
			if (wasConsumed)
			{
				break;
			}
		}
		else
		{
			// Print error message
		}
	}
	m_subscriptionListMutex.unlock();
}


//--------------------------------------------------------------------------------------------------
void EventSystem::FireEvent(std::string const& eventName)
{
	EventArgs emptyArgs;
	FireEvent(eventName, emptyArgs);
}


//--------------------------------------------------------------------------------------------------
void EventSystem::GetAllRegisteredCommands(std::vector<std::string>& out_registeredCommandList)
{
	// for iterator and then use .first()
	m_subscriptionListMutex.lock();
	std::map<std::string, SubscriptionList>::iterator eventIter;
	for (eventIter = m_subscriptionListByEventName.begin(); eventIter != m_subscriptionListByEventName.end(); ++eventIter)
	{
		out_registeredCommandList.push_back(eventIter->first);
	}
	m_subscriptionListMutex.unlock();
}


//--------------------------------------------------------------------------------------------------
void SubscribeEventCallbackFunction(std::string const& eventName, EventCallbackFunction functionPtr)
{
	g_theEventSystem->SubscribeEventCallbackFunction(eventName, functionPtr);
}


//--------------------------------------------------------------------------------------------------
void UnsubscribeEventCallbackFunction(std::string const& eventName, EventCallbackFunction functionPtr)
{
	g_theEventSystem->UnsubscribeEventCallbackFunction(eventName, functionPtr);
}


//--------------------------------------------------------------------------------------------------
void UnsubscribeFromAllEvents(EventCallbackFunction functionPtr)
{
	g_theEventSystem->UnsubscribeFromAllEvents(functionPtr);
}


//--------------------------------------------------------------------------------------------------
void FireEvent(std::string const& eventName, EventArgs& args)
{
	g_theEventSystem->FireEvent(eventName, args);
}


//--------------------------------------------------------------------------------------------------
void FireEvent(std::string const& eventName)
{
	g_theEventSystem->FireEvent(eventName);
}
