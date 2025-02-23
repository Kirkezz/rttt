#ifndef EVENTLISTENER_HPP
#define EVENTLISTENER_HPP

#include <RmlUi/Core.h>

class RTTT_Engine;

class ElementsEventListener : public Rml::EventListener {
  public:
    RTTT_Engine* engine;
    Rml::Element* element_self;
    std::string value;
    ElementsEventListener(RTTT_Engine* engine, std::string value, Rml::Element* element) : engine(engine), value(std::move(value)), element_self(element) {};

    void ProcessEvent(Rml::Event& event);
};

class EventListener : public Rml::EventListenerInstancer {
  public:
    RTTT_Engine* engine;

    Rml::EventListener* InstanceEventListener(const Rml::String& value, Rml::Element* element);
};

#endif //EVENTLISTENER_HPP
