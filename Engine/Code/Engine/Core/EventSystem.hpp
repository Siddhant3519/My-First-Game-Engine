#pragma once

//--------------------------------------------------------------------------------------------------
#include "Engine/Core/NamedStrings.hpp"


//--------------------------------------------------------------------------------------------------
#include <string>
#include <vector>
#include <mutex>
#include <map>


//--------------------------------------------------------------------------------------------------
typedef NamedStrings EventArgs;
typedef bool (*EventCallbackFunction)(EventArgs& args);
typedef std::vector<EventCallbackFunction>	SubscriptionList;

//--------------------------------------------------------------------------------------------------
class EventSystem;
extern EventSystem* g_theEventSystem;

//--------------------------------------------------------------------------------------------------
struct EventSystemConfig
{
	// TODO: Fill in the struct
};


//--------------------------------------------------------------------------------------------------
class EventSystem
{
public:
	EventSystem(EventSystemConfig const& config) { (void)config; };
	~EventSystem() {};


	void Startup() {};
	void Shutdown() {};
	void BeginFrame() {};
	void EndFrame() {};

	void SubscribeEventCallbackFunction(std::string const& eventName, EventCallbackFunction functionPtr);
	void UnsubscribeEventCallbackFunction(std::string const& eventName, EventCallbackFunction functionPtr);
	void UnsubscribeFromAllEvents(EventCallbackFunction functionPtr);
	void FireEvent(std::string const& eventName, EventArgs& args);
	void FireEvent(std::string const& eventName);

	void GetAllRegisteredCommands(std::vector<std::string>& out_registeredCommandList);

private:
	std::mutex								m_subscriptionListMutex;
	std::map<std::string, SubscriptionList> m_subscriptionListByEventName;
};


//--------------------------------------------------------------------------------------------------
void SubscribeEventCallbackFunction(std::string const& eventName, EventCallbackFunction functionPtr);
void UnsubscribeEventCallbackFunction(std::string const& eventName, EventCallbackFunction functionPtr);
void UnsubscribeFromAllEvents(EventCallbackFunction functionPtr);
void FireEvent(std::string const& eventName, EventArgs& args);
void FireEvent(std::string const& eventName);