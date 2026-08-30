#include <chrono>
#include <memory>
#include <string>

#include <gz/common/Console.hh>
#include <gz/math/Color.hh>
#include <gz/msgs/boolean.pb.h>
#include <gz/msgs/details/boolean.pb.h>
#include <gz/msgs/details/entity.pb.h>
#include <gz/msgs/visual.pb.h> // Required for VisualCmd message
#include <gz/plugin/Register.hh>
#include <gz/sim/EntityComponentManager.hh>
#include <gz/sim/EventManager.hh>
#include <gz/sim/System.hh>
#include <gz/sim/Types.hh>
#include <gz/transport/Node.hh>
#include <sdf/Element.hh>

// Add helper classes for entity traversal
#include <gz/sim/Link.hh>
#include <gz/sim/Model.hh>

// Material and visual properties
#include <gz/sim/components/Material.hh>
#include <gz/sim/components/Visual.hh>
#include <gz/sim/components/VisualCmd.hh> // Required to force rendering updates

#include <led_sensor.hh>

namespace gz {
namespace sim {
namespace systems {

class LedSensor::Implementation {
public:
  Entity entity;
  bool ledstate{false};
  double blinkInterval{0.5};
  std::chrono::steady_clock::duration lastToggleTime{0};

  transport::Node node;
  transport::Node::Publisher pub;
  std::string topic{"/led/state"};
};

LedSensor::LedSensor() : dataptr(std::make_unique<Implementation>()) {}
LedSensor::~LedSensor() = default;

void LedSensor::Configure(const Entity &_entity,
                          const std::shared_ptr<const sdf::Element> &_sdf,
                          EntityComponentManager &_ecm,
                          EventManager &_eventMgr) {
  this->dataptr->entity = _entity;

  if (_sdf->HasElement("topic")) {
    this->dataptr->topic = _sdf->Get<std::string>("topic");
  }

  if (_sdf->HasElement("blink_rate")) {
    double hz = _sdf->Get<double>("blink_rate");
    if (hz > 0.0) {
      this->dataptr->blinkInterval = 1.0 / hz / 2.0;
    }
  }

  this->dataptr->pub =
      this->dataptr->node.Advertise<gz::msgs::Boolean>(this->dataptr->topic);
}

void LedSensor::PostUpdate(const UpdateInfo &_info,
                           const EntityComponentManager &_ecm) {
  if (_info.paused) {
    return;
  }

  auto elapsedSecs = std::chrono::duration<double>(
                         _info.simTime - this->dataptr->lastToggleTime)
                         .count();

  // Guard against initial zero-jump or negative time steps
  if (this->dataptr->lastToggleTime == std::chrono::steady_clock::duration(0) ||
      elapsedSecs >= this->dataptr->blinkInterval) {

    this->dataptr->ledstate = !this->dataptr->ledstate;
    this->dataptr->lastToggleTime = _info.simTime;

    auto &mutableEcm = const_cast<EntityComponentManager &>(_ecm);

    gz::sim::Model model(this->dataptr->entity);
    auto links = model.Links(mutableEcm);

    for (const auto &linkEntity : links) {
      gz::sim::Link link(linkEntity);
      auto visuals = link.Visuals(mutableEcm);

      for (const auto &visEntity : visuals) {
        auto *matComp = mutableEcm.Component<components::Material>(visEntity);
        if (!matComp) {
          matComp = mutableEcm.CreateComponent<components::Material>(
              visEntity, components::Material());
        }

        gz::msgs::Visual visMsg;
        auto *matMsg = visMsg.mutable_material();

        if (this->dataptr->ledstate) {
          // ON State: Full Green
          matComp->Data().SetAmbient(gz::math::Color(0.0f, 1.0f, 0.0f, 1.0f));
          matComp->Data().SetDiffuse(gz::math::Color(0.0f, 1.0f, 0.0f, 1.0f));
          matComp->Data().SetEmissive(gz::math::Color(0.0f, 1.0f, 0.0f, 1.0f));
          matComp->Data().SetSpecular(gz::math::Color(0.5f, 1.0f, 0.5f, 1.0f));

          // Explicitly set all channels to avoid white washout
          matMsg->mutable_ambient()->set_r(0.0f);
          matMsg->mutable_ambient()->set_g(1.0f);
          matMsg->mutable_ambient()->set_b(0.0f);
          matMsg->mutable_ambient()->set_a(1.0f);

          matMsg->mutable_diffuse()->set_r(0.0f);
          matMsg->mutable_diffuse()->set_g(1.0f);
          matMsg->mutable_diffuse()->set_b(0.0f);
          matMsg->mutable_diffuse()->set_a(1.0f);

          matMsg->mutable_emissive()->set_r(0.0f);
          matMsg->mutable_emissive()->set_g(1.0f);
          matMsg->mutable_emissive()->set_b(0.0f);
          matMsg->mutable_emissive()->set_a(1.0f);

          matMsg->mutable_specular()->set_r(0.5f);
          matMsg->mutable_specular()->set_g(1.0f);
          matMsg->mutable_specular()->set_b(0.5f);
          matMsg->mutable_specular()->set_a(1.0f);
        } else {
          // OFF State: Dark Gray
          matComp->Data().SetAmbient(gz::math::Color(0.1f, 0.1f, 0.1f, 1.0f));
          matComp->Data().SetDiffuse(gz::math::Color(0.1f, 0.1f, 0.1f, 1.0f));
          matComp->Data().SetEmissive(gz::math::Color(0.0f, 0.0f, 0.0f, 1.0f));
          matComp->Data().SetSpecular(gz::math::Color(0.1f, 0.1f, 0.1f, 1.0f));

          matMsg->mutable_ambient()->set_r(0.1f);
          matMsg->mutable_ambient()->set_g(0.1f);
          matMsg->mutable_ambient()->set_b(0.1f);
          matMsg->mutable_ambient()->set_a(1.0f);

          matMsg->mutable_diffuse()->set_r(0.1f);
          matMsg->mutable_diffuse()->set_g(0.1f);
          matMsg->mutable_diffuse()->set_b(0.1f);
          matMsg->mutable_diffuse()->set_a(1.0f);

          matMsg->mutable_emissive()->set_r(0.0f);
          matMsg->mutable_emissive()->set_g(0.0f);
          matMsg->mutable_emissive()->set_b(0.0f);
          matMsg->mutable_emissive()->set_a(1.0f);

          matMsg->mutable_specular()->set_r(0.1f);
          matMsg->mutable_specular()->set_g(0.1f);
          matMsg->mutable_specular()->set_b(0.1f);
          matMsg->mutable_specular()->set_a(1.0f);
        }

        mutableEcm.SetChanged(visEntity, components::Material::typeId,
                              ComponentState::OneTimeChange);
        mutableEcm.SetChanged(visEntity, components::Visual::typeId,
                              ComponentState::OneTimeChange);

        auto *cmdComp = mutableEcm.Component<components::VisualCmd>(visEntity);
        if (!cmdComp) {
          mutableEcm.CreateComponent(visEntity, components::VisualCmd(visMsg));
        } else {
          cmdComp->Data() = visMsg;
          mutableEcm.SetChanged(visEntity, components::VisualCmd::typeId,
                                ComponentState::OneTimeChange);
        }
      }
    }

    gz::msgs::Boolean msg;
    auto secs = std::chrono::floor<std::chrono::seconds>(_info.simTime);
    auto nsecs = std::chrono::duration_cast<std::chrono::nanoseconds>(
        _info.simTime - secs);

    msg.mutable_header()->mutable_stamp()->set_sec(secs.count());
    msg.mutable_header()->mutable_stamp()->set_nsec(nsecs.count());
    msg.set_data(this->dataptr->ledstate);

    this->dataptr->pub.Publish(msg);
  }
}

} // namespace systems
} // namespace sim
} // namespace gz

GZ_ADD_PLUGIN(gz::sim::systems::LedSensor, gz::sim::System,
              gz::sim::ISystemConfigure, gz::sim::ISystemPostUpdate)