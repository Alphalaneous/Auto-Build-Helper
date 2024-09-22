#include <Geode/Geode.hpp>
#include <Geode/modify/EditorUI.hpp>
#include <Geode/modify/LevelEditorLayer.hpp>
#include <Geode/modify/EditorPauseLayer.hpp>

using namespace geode::prelude;

class $modify(MyEditorUI, EditorUI) {

	struct Fields {
		Ref<EditorPauseLayer> m_pauseLayer;
		bool m_autoBuildHelperEnabled = false;
	};

	bool init(LevelEditorLayer* editorLayer) {

		if (!EditorUI::init(editorLayer)) return false;

		m_fields->m_pauseLayer = EditorPauseLayer::create(editorLayer);
		m_fields->m_pauseLayer->setTouchEnabled(false);
		m_fields->m_pauseLayer->setKeyboardEnabled(false);
		m_fields->m_pauseLayer->setKeypadEnabled(false);

		CCTouchDispatcher::get()->unregisterForcePrio(m_fields->m_pauseLayer);
		CCTouchDispatcher::get()->removeDelegate(m_fields->m_pauseLayer);

		queueInMainThread([this] {
			if (auto delegate = typeinfo_cast<CCTouchDelegate*>(m_fields->m_pauseLayer.data())) {
				if (auto handler = CCTouchDispatcher::get()->findHandler(delegate)) {
					CCTouchDispatcher::get()->setPriority(100000, handler->getDelegate());
				}
			}
		});

		if (!Mod::get()->getSettingValue<bool>("show-on-pause")) {
			if (CCMenu* menu = typeinfo_cast<CCMenu*>(getChildByID("toolbar-toggles-menu"))) {

				CCSprite* autoBuildHelperSpr = CCSprite::create("build_helper.png"_spr);
				
				ButtonSprite* autoBuildHelperSprOn = ButtonSprite::create(autoBuildHelperSpr, 40, true, 40, "GJ_button_02.png", 1);
				ButtonSprite* autoBuildHelperSprOff = ButtonSprite::create(autoBuildHelperSpr, 40, true, 40, "GJ_button_01.png", 1);

				autoBuildHelperSpr->setPositionY(autoBuildHelperSpr->getPositionY()-2);

				autoBuildHelperSprOn->setContentSize({40, 40});
				autoBuildHelperSprOff->setContentSize({40, 40});

				CCMenuItemToggler* toggler = CCMenuItemToggler::create(autoBuildHelperSprOn, autoBuildHelperSprOff, this, menu_selector(MyEditorUI::onToggleAutoBuildHelper));
				toggler->setID("build-helper-button"_spr);
				toggler->toggle(true);
				menu->addChild(toggler);
				menu->updateLayout();
			}
		}

		return true;
	}

	void onToggleAutoBuildHelper(CCObject* sender) {
		CCMenuItemToggler* toggler = static_cast<CCMenuItemToggler*>(sender);
		m_fields->m_autoBuildHelperEnabled = toggler->isOn();
		// editor sounds compat
		if (!Mod::get()->getSettingValue<bool>("show-on-pause")) {
			sender->setTag(m_selectedMode);
			EditorUI::toggleMode(sender);
		}
	}
};

class $modify(MyEditorPauseLayer, EditorPauseLayer) {

    bool init(LevelEditorLayer* p0) {
		if (!EditorPauseLayer::init(p0)) return false;

		if (Mod::get()->getSettingValue<bool>("show-on-pause")) {
			if (CCMenu* menu = typeinfo_cast<CCMenu*>(getChildByID("guidelines-menu"))) {

				CCSprite* autoBuildHelperSpr = CCSprite::create("build_helper.png"_spr);
				
				CircleButtonSprite* autoBuildHelperSprOn = CircleButtonSprite::create(autoBuildHelperSpr, CircleBaseColor::Cyan, CircleBaseSize::Small);
				CircleButtonSprite* autoBuildHelperSprOff = CircleButtonSprite::create(autoBuildHelperSpr, CircleBaseColor::Gray, CircleBaseSize::Small);

				autoBuildHelperSprOn->setContentSize({40, 40});
				autoBuildHelperSprOff->setContentSize({40, 40});

				CCMenuItemToggler* toggler = CCMenuItemToggler::create(autoBuildHelperSprOn, autoBuildHelperSprOff, EditorUI::get(), menu_selector(MyEditorUI::onToggleAutoBuildHelper));
				toggler->setID("build-helper-button"_spr);
				toggler->toggle(true);
				menu->addChild(toggler);
				menu->updateLayout();
			}
		}

		return true;
	}
};

class $modify(MyLevelEditorLayer, LevelEditorLayer) {

    cocos2d::CCArray* createObjectsFromString(gd::string const& p0, bool p1, bool p2){
		auto ret = LevelEditorLayer::createObjectsFromString(p0, p1, p2);
		
		if (m_editorUI) {
			MyEditorUI* mui = static_cast<MyEditorUI*>(m_editorUI);
			if (!p1 && !p2 && mui->m_fields->m_autoBuildHelperEnabled){
				queueInMainThread([mui] {
					mui->m_fields->m_pauseLayer->onBuildHelper(mui->m_fields->m_pauseLayer);
				});
			}
		}

		return ret;
	}
};