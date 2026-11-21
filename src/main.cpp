#include <Geode/Geode.hpp>
#include <Geode/binding/GJBaseGameLayer.hpp>
#include <Geode/binding/PlayerObject.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/LevelEditorLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>

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


class $modify(PEIELevelEditorLayer, LevelEditorLayer) {
	void onPlaytest() {
		LevelEditorLayer::onPlaytest();
		FMODAudioEngine::get()->m_metering = true;
		FMODAudioEngine::get()->m_pulse1 = 0.1f;
		FMODAudioEngine::get()->m_pulse2 = 0.1f;
		FMODAudioEngine::get()->m_pulse3 = 0.0f;
	}
	
	void onStopPlaytest() {
		LevelEditorLayer::onStopPlaytest();
		FMODAudioEngine::get()->m_metering = false;
	}
};


class $modify(PEIEPlayerOject, PlayerObject) {
	static void onModify(auto& self) {
		(void)self.setHookPriorityPre("PlayerObject::activateStreak", Priority::Replace);
	}
	
    void activateStreak() {
        if (!this->levelFlipping()) {
            if (!m_isHidden) {
                this->m_fadeOutStreak = true;
                this->m_regularTrail->resumeStroke();
                if (m_isDart) {
                    m_waveTrail->m_currentPoint = getPosition();
                    m_waveTrail->stopAllActions();
                    m_waveTrail->setOpacity(255);
					#ifdef GEODE_IS_MACOS
						m_waveTrail->resumeStroke();
					#else
						m_waveTrail->m_drawStreak = true;
						m_waveTrail->updateStroke(0.0f);
					#endif
                }
            }
        }
    } // basically 1:1 recreation from ghidra decompile, minus the "GameManger::get()->m_isEditor" check yayayaya
	
    void playSpiderDashEffect(CCPoint from, CCPoint to) {
		PlayerObject::playSpiderDashEffect(from, to);
		// if i add the circle waves later, then i'll just replace the main function
		if (GJBaseGameLayer::get()->m_isEditor && !m_isHidden) {
			auto color = this->m_playerColor1;
			CCLayer* objectLayer = GJBaseGameLayer::get()->m_objectLayer;
		
			if (!m_swapColors) {
                color = this->m_playerColor2;
            }
		
			#ifdef GEODE_IS_MACOS
				this->flashPlayer(0.3, 0.5, ccWHITE, ccWHITE); 
			#else
				this->m_flashMainColor = ccColor3B(255, 255, 255);
				this->m_flashSecondColor = ccColor3B(255, 255, 255);
				this->setColor(m_flashMainColor);
				this->setSecondColor(m_flashSecondColor);
				this->m_flashTime = m_totalTime;
				this->m_flashDuration = 0.3;
				this->m_flashDelay = 0.05;
			#endif
			
			this->m_spiderSprite->stopActionByTag(10);
			this->m_spiderSprite->setScale(0.4);
			CCScaleTo* scaleAction = CCScaleTo::create(0.45, 1.0);
			CCEaseElasticOut* scaleEasing = CCEaseElasticOut::create(scaleAction, 0.35);
			this->m_spiderSprite->runAction(scaleEasing);
			
			
			CCPoint difference = ccp(to.x - from.x, to.y - from.y);
			CCPoint spritePos;
			float spriteLength;
			CCSprite* spiderDashSprite = CCSprite::createWithSpriteFrameName("spiderDash_002.png");
			spiderDashSprite->setColor(color);
			objectLayer->addChild(spiderDashSprite, 40);
			if (!m_isSideways) {
				spritePos.x = from.x;
				spritePos.y = (to.y - from.y) * 0.5 + from.y;
				spriteLength = std::abs(difference.y) + (m_vehicleSize * 30.f);
				if (from.y > to.y) {
					spiderDashSprite->setRotation(-90.f);
				}
				else {
					spiderDashSprite->setRotation(90.f);
				}	
			}
			else {
				spritePos.x = (to.x - from.x) * 0.5 + from.x;
				spritePos.y = from.y;
				spriteLength = std::abs(difference.x) + (m_vehicleSize * 30.f);
				if (from.x < to.x) {
					spiderDashSprite->setRotation(180.f);
				}
			}
			spiderDashSprite->setPosition(spritePos);
			CCSize size = spiderDashSprite->getContentSize();
			spiderDashSprite->setScaleX(spriteLength / size.width);
			spiderDashSprite->setScaleY(m_vehicleSize * 1.2);
			// it's funny because when making this i realized rob's code doesn't account for arrow triggers correctly
			// with his code, you'll see the sprite shifted to the right while sideways
			// here i have that fixed (i'm not replacing the function right now so idc)
			CCArray* frames = CCArray::create();
			for (int i = 1; i < 9; i++) {
				CCString* frameName = CCString::createWithFormat("spiderDash_%03d.png", i);
				CCSpriteFrame* frame = CCSpriteFrameCache::sharedSpriteFrameCache()->spriteFrameByName(frameName->getCString());
				frames->addObject(frame);
			}
			CCAnimation* spiderDashAnimation = CCAnimation::createWithSpriteFrames(frames, 0.04f);
			CCAnimate* spiderDashAnimate = CCAnimate::create(spiderDashAnimation);
			CCCallFunc* removeCall = CCCallFunc::create(spiderDashSprite, callfunc_selector(CCNode::removeMeAndCleanup));
			CCSequence* spiderDashSequence = CCSequence::create(spiderDashAnimate, removeCall, 0);
			spiderDashSprite->runAction(spiderDashSequence);
			if ((!m_isHidden) && (!m_isSpider || !m_isPlatformer)) {
				this->m_playerGroundParticles->resetSystem();
			}
		}
        return;
    } // not worrying about the circle waves for now
	
	void startDashing(DashRingObject* object) {
		PlayerObject::startDashing(object);
		if (GJBaseGameLayer::get()->m_isEditor && !m_isHidden) {
			#ifdef GEODE_IS_MACOS
				this->flashPlayer(0.2, 0.1, ccWHITE, ccWHITE); 
			#else
				this->m_flashMainColor = ccColor3B(255, 255, 255);
				this->m_flashSecondColor = ccColor3B(255, 255, 255);
				this->setColor(m_flashMainColor);
				this->setSecondColor(m_flashSecondColor);
				this->m_flashTime = m_totalTime;
				this->m_flashDuration = 0.2;
				this->m_flashDelay = 0.1;
			#endif
		}
	}
	
	void update(float dt) {
        PlayerObject::update(dt);
		if (LevelEditorLayer::get()) {
			float pulse = FMODAudioEngine::get()->m_pulse1;
			this->m_waveTrail->m_waveSize = 10 * pulse * m_vehicleSize;
		}
	}
};