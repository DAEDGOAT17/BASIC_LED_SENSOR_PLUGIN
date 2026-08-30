#ifndef LED_SENSOR_HH_
#define LED_SENSOR_HH_

// this is mostly boikerplate code  in the system 

#include <gz/sim/Entity.hh>
#include <gz/sim/EntityComponentManager.hh>
#include <gz/sim/EventManager.hh>
#include <gz/sim/System.hh>
#include <gz/sim/Types.hh>
#include <memory.h>
#include <memory>
#include <sdf/Element.hh>

namespace gz {
namespace sim {
namespace systems {

class LedSensor : public System,
                  public ISystemConfigure,
                  public ISystemPostUpdate {
public:
  LedSensor();
  ~LedSensor() override;
  //first interface to turn code to the plugin 
    void Configure (
    const Entity &_entity ,
    const std::shared_ptr<const sdf::Element> &_sdf ,
    EntityComponentManager &_ecm ,
    EventManager &_eventMgr ) override ;
  // second interface for the same 
    void PostUpdate(const UpdateInfo &_info ,const EntityComponentManager &_ecm) override ;

private:
    class Implementation ;
    std::unique_ptr<Implementation> dataptr ;
} ;

} // namespace systems
} // namespace sim
} // namespace gz

#endif
