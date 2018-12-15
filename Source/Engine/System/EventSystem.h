#include <memory>
#include <list>
#include <map>
#include <Windows.h>
#include "FastDelegate.h"

// {882A2494-4A68-495E-B0B3-C1DED01518C5}
DEFINE_GUID(GUID_DEFINED, 0x882a2494, 0x4a68, 0x495e, 0xb0, 0xb3, 0xc1, 0xde, 0xd0, 0x15, 0x18, 0xc5);

typedef unsigned long EventType;

class IEventData {
public:
	virtual const EventType& VGetEventType(void) const = 0;
	virtual float VGetTimeStamp(void) const = 0;
	virtual const char* GetName(void) const = 0;
};

typedef std::shared_ptr<IEventData> IEventDataPtr;

class BaseEventData : public IEventData {
	const float m_timeStamp;
public:
	explicit BaseEventData(const float timeStamp = 0.0f) : m_timeStamp(timeStamp) {}
	virtual ~BaseEventData(void) {}
	virtual const EventType& VGetEventType(void) const = 0;
	float VGetTimeStamp(void) const { return m_timeStamp; }
};

class EvtData_Mouse_Clicked : public BaseEventData {
	float mouseX;
	float mouseY;
public:
	static const EventType sk_EventType;
	explicit EvtData_Mouse_Clicked(float x, float y) : mouseX(x), mouseY(y) { }
	virtual const EventType& VGetEventType(void) const {
		return sk_EventType;
	}
	virtual const char* GetName(void) const {
		return "EvtData_Mouse_Clicked";
	}
	float GetX(void) const { return mouseX; }
	float GetY(void) const { return mouseY; }
};

const EventType EvtData_Mouse_Clicked::sk_EventType(0x882a2494);

typedef fastdelegate::FastDelegate1<IEventDataPtr> EventListenerDelegate;

class IEventManager {
public:
	enum eConstants { kINFINITE = 0xffffffff};

	virtual bool VAddListener(const EventListenerDelegate& eventDelegate, const EventType& type) = 0;
	virtual bool VRemoveListener(const EventListenerDelegate& eventDelegate, const EventType& type) = 0;
	//virtual bool VTriggerEvent(const IEventDataPtr& pEvent) const = 0;
	virtual bool VQueueEvent(const IEventDataPtr& pEvent) = 0;
	virtual bool VAbortEvent(const EventType& type, bool allOfType = false) = 0;
	virtual bool VTickUpdate(unsigned long maxMillis = kINFINITE) = 0;
};

const unsigned int EVENTMANAGER_NUM_QUEUES = 2;
class EventManager : public IEventManager {
	typedef std::list<EventListenerDelegate> EventListenerList;
	typedef std::map<EventType, EventListenerList> EventListenerMap;
	typedef std::list<IEventDataPtr> EventQueue;

	EventListenerMap m_eventListeners;
	EventQueue m_queues[EVENTMANAGER_NUM_QUEUES];
	int m_activeQueue;

public:
	static EventManager* instance;

	explicit EventManager(const char* pName, bool setAsGlobal) {
		if (instance == nullptr) {
			instance = this;
		}
	}
	virtual ~EventManager(void) {}
	virtual bool VAddListener(const EventListenerDelegate& eventDelegate, const EventType& type);
	virtual bool VRemoveListener(const EventListenerDelegate& eventDelegate, const EventType& type);
	//virtual bool VTriggerEvent(const IEventDataPtr& pEvent) const;
	virtual bool VQueueEvent(const IEventDataPtr& pEvent);
	virtual bool VAbortEvent(const EventType& type, bool allOfType = false);
	virtual bool VTickUpdate(unsigned long maxMillis = kINFINITE);
};

EventManager* EventManager::instance;

bool EventManager::VAddListener(const EventListenerDelegate& eventDelegate, const EventType& type) {
	EventListenerList& eventListenerList = m_eventListeners[type];

	for (auto it = eventListenerList.begin(); it != eventListenerList.end(); ++it) {
		if (eventDelegate == (*it)) {
			return false;
		}
	}

	eventListenerList.push_back(eventDelegate);
	return true;
}

bool EventManager::VRemoveListener(const EventListenerDelegate & eventDelegate, const EventType & type){
	bool success = false;
	auto findIt = m_eventListeners.find(type);
	if (findIt != m_eventListeners.end()) {
		EventListenerList& listeners = findIt->second;
		for (auto it = listeners.begin(); it != listeners.end(); ++it) {
			if (eventDelegate == (*it)) {
				listeners.erase(it);
				success = true;
				break;
			}
		}
	}
	return success;
}

bool EventManager::VQueueEvent(const IEventDataPtr & pEvent){
	auto findIt = m_eventListeners.find(pEvent->VGetEventType());
	if (findIt != m_eventListeners.end()) {
		m_queues[m_activeQueue].push_back(pEvent);
		return true;
	}
	return false;
}

bool EventManager::VAbortEvent(const EventType & type, bool allOfType){
	bool success = false;
	EventListenerMap::iterator findIt = m_eventListeners.find(type);
	if (findIt != m_eventListeners.end()) {
		EventQueue& eventQueue = m_queues[m_activeQueue];
		auto it = eventQueue.begin();
		while (it != eventQueue.end()) {
			auto thisIt = it;
			++it;
			if ((*thisIt)->VGetEventType() == type) {
				eventQueue.erase(thisIt);
				success = true;
				if (!allOfType) {
					return success;
				}
			}
		}
	}
	return success;
}

bool EventManager::VTickUpdate(unsigned long maxMillis){
	unsigned long currMs = GetTickCount();
	unsigned long maxMs = ((maxMillis == IEventManager::kINFINITE) ? (IEventManager::kINFINITE) : (currMs + maxMillis));

	int queueToProcess = m_activeQueue;
	m_activeQueue = (m_activeQueue + 1) % EVENTMANAGER_NUM_QUEUES;
	m_queues[m_activeQueue].clear();

	while (!m_queues[queueToProcess].empty()) {
		IEventDataPtr pEvent = m_queues[queueToProcess].front();
		m_queues[queueToProcess].pop_front();
		const EventType& eventType = pEvent->VGetEventType();

		auto findIt = m_eventListeners.find(eventType);
		if (findIt != m_eventListeners.end()) {
			const EventListenerList& eventListeners = findIt->second;
			for (auto it = eventListeners.begin(); it != eventListeners.end(); ++it) {
				EventListenerDelegate listener = (*it);
				listener(pEvent);
			}
		}

		currMs = GetTickCount();

		if (maxMillis != IEventManager::kINFINITE && currMs >= maxMs) {
			break;
		}
	}

	bool queueFlushed = (m_queues[queueToProcess].empty());
	if (!queueFlushed) {
		while (!m_queues[queueToProcess].empty()) {
			IEventDataPtr pEvent = m_queues[queueToProcess].back();
			m_queues[queueToProcess].pop_back();
			m_queues[m_activeQueue].push_front(pEvent);
		}
	}
	return queueFlushed;
}