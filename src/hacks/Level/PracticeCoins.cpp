#include <modules/config/config.hpp>
#include <modules/gui/gui.hpp>
#include <modules/gui/components/toggle.hpp>
#include <modules/hack/hack.hpp>

#include <Geode/modify/GJBaseGameLayer.hpp>

namespace eclipse::hacks::Level {
    class $hack(PracticeCoins) {
        void init() override {
            auto tab = gui::MenuTab::find("tab.level");
            tab->addToggle("level.practicecoins")->setDescription()->handleKeybinds();
        }

        [[nodiscard]] const char* getId() const override { return "Practice Coins"; }
    };

    REGISTER_HACK(PracticeCoins)

    class $modify(PracticeCoinsBGLHook, GJBaseGameLayer) {
        ADD_HOOKS_DELEGATE("level.practicecoins")

        void collisionCheckObjects(PlayerObject* player, gd::vector<GameObject*>* sectionObjects, int objectCount, float dt) {
            if (!m_isPracticeMode)
                return GJBaseGameLayer::collisionCheckObjects(player, sectionObjects, objectCount, dt);

            auto playerRect = player->getObjectRect();
            for (int i = 0; i < objectCount; i++) {
                auto* obj = sectionObjects->at(i);

                // check if the object is a coin
                if (obj->m_objectType != GameObjectType::SecretCoin && obj->m_objectType != GameObjectType::UserCoin)
                    continue;

                auto* effectSprite = geode::cast::typeinfo_cast<EffectGameObject*>(obj);
                if (!effectSprite) // weird, but just in case
                    continue;

                // check if the coin was already collected
                if (effectSprite->getOpacity() == 0)
                    continue;

                // check if the player is colliding with the object
                auto objectRect = effectSprite->getObjectRect();
                if (!playerRect.intersectsRect(objectRect))
                    continue;

                // reconstruction of the original code
                effectSprite->EffectGameObject::triggerObject(this, player->m_uniqueID, nullptr); // not sure if needed
                GJBaseGameLayer::destroyObject(effectSprite); // plays the coin collect animation
                // if (!GJBaseGameLayer::hasUniqueCoin(effectSprite))
                //     GJBaseGameLayer::pickupItem(effectSprite); // not required, because the coin is not saved anyway
                GJBaseGameLayer::gameEventTriggered(GJGameEvent::UserCoin, 0, 0); // event triggers
            }

            GJBaseGameLayer::collisionCheckObjects(player, sectionObjects, objectCount, dt);
        }
    };
}
