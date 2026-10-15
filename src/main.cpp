//#include "Geode/cocos/actions/CCActionEase.h"
//#include "Geode/cocos/actions/CCActionInterval.h"
//#include "ccTypes.h"
//#include <Geode/Enums.hpp>
#include <Geode/Geode.hpp>
#include <Geode/binding/GJBaseGameLayer.hpp>
//#include <Geode/binding/GameManager.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/binding/PlayerObject.hpp>
//#include <Geode/binding/CCCircleWave.hpp>

using namespace geode::prelude;

class $modify(PEIEGJBaseGameLayer, GJBaseGameLayer) {
    void createPlayer() {
        GJBaseGameLayer::createPlayer();
        if (m_player1) {
            if (m_isEditor) {
                m_player1->addAllParticles();
            }
        }
    }
    void toggleDualMode(GameObject* object, bool dual, PlayerObject* player, bool noEffects) {
        GJBaseGameLayer::toggleDualMode(object, dual, player, noEffects);
        if (m_isEditor) {
            if (m_gameState.m_isDualMode) {
                m_player2->addAllParticles();
            }
        }
    }
};


class $modify(PEIEPlayerOject, PlayerObject) {
    void activateStreak() {
        if (!this->levelFlipping()) {
            if (!m_isHidden) {
                this->m_fadeOutStreak = true;
                this->m_regularTrail->resumeStroke();
                if (m_isDart) {
                    m_waveTrail->m_currentPoint = getPosition();
                    m_waveTrail->stopAllActions();
                    m_waveTrail->setOpacity(255);
                    m_waveTrail->m_drawStreak = true;
                    m_waveTrail->updateStroke(0.0f);

                }
            }
        }
    } // basically 1:1 recreation from ghidra decompile, minus the "GameManger::get()->m_isEditor" check yayayaya
};