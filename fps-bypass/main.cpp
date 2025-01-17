#include <Geode/Bindings.hpp>
#include <Geode/modify/AppDelegate.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/loader/SettingEvent.hpp>
#include <Geode/loader/ModJsonTest.hpp>
#include <chrono>

using namespace geode::prelude;

void update_fps() {
	if (Mod::get()->getSettingValue<bool>("enabled")) {
		auto* app = CCApplication::sharedApplication();
		if (app->getVerticalSyncEnabled()) {
			app->toggleVerticalSync(false);
			GameManager::sharedState()->setGameVariable("0030", false);
		}
		const int value = Mod::get()->getSettingValue<int64_t>("fps-value");
		app->setAnimationInterval(1.0 / static_cast<double>(value));
	} else {
		CCApplication::sharedApplication()->setAnimationInterval(1.0 / 60.0);
	}
}

class $modify(AppDelegate) {
	void applicationWillEnterForeground() {
		AppDelegate::applicationWillEnterForeground();
		update_fps();
	}
};

$execute {
	listenForSettingChanges("fps-value", +[](int64_t) {
		update_fps();
	});
	listenForSettingChanges("enabled", +[](bool) {
		update_fps();
	});
}

$on_mod(Enabled) {
	update_fps();
}

$on_mod(Disabled) {
	CCApplication::sharedApplication()->setAnimationInterval(1.0 / 60.0);
}

// ui

std::chrono::time_point<std::chrono::high_resolution_clock> previous_frame, last_update;
float frame_time_sum = 0.f;
int frame_count = 0;

class $modify(MyPlayLayer, PlayLayer) {
	CCLabelBMFont* fps_label;

	bool init(GJGameLevel* level) {
		if (!PlayLayer::init(level)) return false;

		if (Mod::get()->getSettingValue<bool>("fps-label")) {
			auto* label = CCLabelBMFont::create("69 fps", "bigFont.fnt");
			m_fields->fps_label = label;
			this->addChild(label);
			
			const auto win_size = CCDirector::sharedDirector()->getWinSize();
			label->setPosition(win_size.width - 3.f, win_size.height - 3.f);
			label->setAnchorPoint(ccp(1, 0.8));
			label->setZOrder(999);
			label->setOpacity(128);
			label->setScale(0.4f);
			label->setTag(1234567);

			this->schedule(schedule_selector(MyPlayLayer::update_label));
		}

		return true;
	}

	void update_label(float) {
		const auto now = std::chrono::high_resolution_clock::now();

		const std::chrono::duration<float> diff = now - previous_frame;
		frame_time_sum += diff.count();
		frame_count++;

		if (std::chrono::duration<float>(now - last_update).count() > 1.0f) {
			last_update = now;
			const auto fps = static_cast<int>(std::roundf(static_cast<float>(frame_count) / frame_time_sum));
			frame_time_sum = 0.f;
			frame_count = 0;
			m_fields->fps_label->setString((std::to_string(fps) + " FPS").c_str());
		}

		previous_frame = now;
	}
};