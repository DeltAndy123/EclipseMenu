#include <cmath>
#include <modules/config/config.hpp>
#include <modules/gui/color.hpp>
#include <modules/gui/gui.hpp>
#include <modules/gui/components/toggle.hpp>
#include <modules/hack/hack.hpp>

#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/HardStreak.hpp>
#include <Geode/modify/CCDrawNode.hpp>

using namespace geode::prelude;

namespace eclipse::hacks::Player {
    class $hack(CustomWaveTrail) {
        void init() override {
            auto tab = gui::MenuTab::find("tab.player");

            config::setIfEmpty("player.customwavetrail.scale", 2.f);
            config::setIfEmpty("player.customwavetrail.speed", 0.5f);
            config::setIfEmpty("player.customwavetrail.saturation", 100.f);
            config::setIfEmpty("player.customwavetrail.value", 100.f);
            config::setIfEmpty("player.customwavetrail.color", gui::Color::WHITE);
            config::setIfEmpty("player.customwavetrail.outline.color", gui::Color::BLACK);
            config::setIfEmpty("player.customwavetrail.outline.stroke", 2.f);

            tab->addToggle("player.customwavetrail")->handleKeybinds()->setDescription()
               ->addOptions([](auto options) {
                   options->addInputFloat("player.customwavetrail.scale", 0.f, 10.f, "%.2f");
                   options->addToggle("player.customwavetrail.rainbow")->addOptions([](auto opt) {
                       opt->addInputFloat("player.customwavetrail.speed", 0.f, FLT_MAX, "%.2f");
                       opt->addInputFloat("player.customwavetrail.saturation", 0.f, 100.f, "%.2f");
                       opt->addInputFloat("player.customwavetrail.value", 0.f, 100.f, "%.2f");
                   });
                   options->addToggle("player.customwavetrail.customcolor")->addOptions([](auto opt) {
                       opt->addColorComponent("player.customwavetrail.color");
                   });
                   options->addToggle("player.customwavetrail.outline")->addOptions([](auto opt) {
                       opt->addInputFloat("player.customwavetrail.outline.stroke", 0.f, 10.f, "%.2f");
                       opt->addColorComponent("player.customwavetrail.outline.color");
                   });
               });
        }

        [[nodiscard]] const char* getId() const override { return "Custom Wave Trail"; }
    };

    REGISTER_HACK(CustomWaveTrail)

    class $modify(WaveTrailSizeHSHook, HardStreak) {
        ADD_HOOKS_DELEGATE("player.customwavetrail")
/*
        double normalizeAngle2(double p1) {
            if (360.0 < p1) {
                return p1 - 360.0;
            } else if (0.0 <= p1) {
                return p1;
            } else {
                return p1 + 360.0;
            }
        }
        cocos2d::CCPoint quadOffset(cocos2d::CCPoint p1, cocos2d::CCPoint p2, float p3) {
            if (1.0F <= p3) {
                double dVar2 = normalizeAngle2(
                    (std::atan2((p2.y - p1.y), (p2.x - p1.x)) * 180.0) / M_PI + 90.0
                );
                float fVar3 = ((float)(dVar2 * M_PI) / 180.F);
                return cocos2d::CCPoint(p3 * 0.5f * std::cosf(fVar3), p3 * 0.5f * std::sinf(fVar3));
            } else {
                return cocos2d::CCPoint();
            }
        }
*/
        void updateStroke(float dt) {
            if (config::get<"player.customwavetrail.rainbow", bool>(false)) {
                auto speed = config::get<"player.customwavetrail.speed", float>(0.5f);
                auto saturation = config::get<"player.customwavetrail.saturation", float>(100.f);
                auto value = config::get<"player.customwavetrail.value", float>(100.f);
                this->setColor(utils::getRainbowColor(speed / 10.f, saturation / 100.f, value / 100.f).toCCColor3B());
            } else if (config::get<"player.customwavetrail.customcolor", bool>(false)) {
                auto color = config::get<"player.customwavetrail.color", gui::Color>(gui::Color::WHITE);
                this->setColor(color.toCCColor3B());
            }

            this->m_pulseSize = config::get<"player.customwavetrail.scale", float>(2.f);

            HardStreak::updateStroke(dt);
        }
    };

    class $modify(WaveTrailStrokeHSHook, cocos2d::CCDrawNode) {
        ADD_HOOKS_DELEGATE("player.customwavetrail.outline")
        bool drawPolygon(cocos2d::CCPoint *verts, unsigned int count, const cocos2d::ccColor4F &fillColor, float borderWidth, const cocos2d::ccColor4F &borderColor) {
            if (fillColor.r == 1.F && fillColor.g == 1.F && fillColor.b == 1.F && fillColor.a != 1.F) return CCDrawNode::drawPolygon(verts, count, fillColor, borderWidth, borderColor);
            auto color = config::get<"player.customwavetrail.outline.color", gui::Color>(gui::Color::BLACK);
            auto width = config::get<"player.customwavetrail.outline.stroke", float>(2.F);
            this->setBlendFunc(cocos2d::CCSprite::create()->getBlendFunc());
            this->setZOrder(-1);
            std::vector<CCPoint> newVerts(count);
            for (unsigned int i = 0; i < count; i++) {
                newVerts[i] = verts[i];
            }
            float offset = width + (width / count);
            newVerts[0].y -= offset;
            newVerts[3].y -= offset;
            newVerts[1].y += offset;
            newVerts[2].y += offset;
            this->drawSegment(newVerts[0], newVerts[3], width, color);
            this->drawSegment(newVerts[1], newVerts[2], width, color);
            return CCDrawNode::drawPolygon(verts, count, fillColor, borderWidth, borderColor);
        }
    };
}
