/**
* @file Main.cpp
*/
#include "MainScene.h"
#include "TitleScene.h"
#include "GameOverScene.h"
#include "GameClearScene.h"
#include "GameData.h"
#include "Actor.h"
#include "GLFWEW.h"
#include "Texture.h"
#include "Sprite.h"
#include "Font.h"
#include "TiledMap.h"
#include "Audio.h"
#include <glm/gtc/constants.hpp>
#include <random>


const char windowTitle[] = "OpenGL2D 2018";	// タイトルバーに表示される文章.


std::mt19937 random;				// 乱数を発生させる変数(乱数エンジン).
int score;							// プレイヤーの得点.
float enemyGenerationTimer;			// 次の敵が出現するまでの時間(単位:秒).
float waitTimer;					// GameOverSceneに移行するまでの時間

const int weaponLevelMin = 1;		// 自機の武器強化の最低段階.
const int weaponLevelMax = 7;		// 自機の武器強化の最高段階.
int weaponLevel;					// 自機武器強化段階.

int laserLevel = 5;					// レーザー使用可能段階.

const int weaponTypeWideShot = 0;	// 広範囲ショット.
const int weaponTypeLaser = 1;		// レーザー.
int weaponType;						// 選択中の武器.

// ゲームの状態.
int gamestate;						// 実行中の場面ID.
Actor* boss;						// ボスActorのポインタ.
bool isStagePassed;					// ステージをクリアしていればtrue.

TitleScene titleScene;
MainScene mainScene;
GameOverScene gameOverScene;
GameClearScene gameClearScene;

// 敵の出現を制御するためのデータ.
TiledMap enemyMap;
float mapCurrentPosX;
float mapProcessedX;
bool isEndOfMap;					// マップの終端に到達したらtrue.

// 音声制御用変数.
Audio::SoundPtr bgm;
Audio::SoundPtr sePlayerShot;
Audio::SoundPtr sePlayerLaser;
Audio::SoundPtr seBlast;
Audio::SoundPtr seBreak;
Audio::SoundPtr sePowerUp;

const float playerWidth = 64;		// 自機の幅.
const float playerHeight = 32;		// 自機の高さ.

SpriteRenderer renderer;			// スプライトを描画するオブジェクト.
FontRenderer fontRenderer;			// フォント描画用変数.
Sprite sprBackground;				// 背景用スプライト.
Actor sprPlayer;					// 自機用Actor配列.
glm::vec3 playerVelocity;			// 自機の移動速度.
Actor enemyList[128];				// 敵のリスト.
Actor playerBulletList[128];		// 自機の弾のリスト.
Actor playerLaserList[6];			// 自機のレーザーのリスト.
Actor effectList[128];				// 爆発などの特殊効果用スプライトのリスト.
Actor itemList[64];					// アイテム用スプライトのリスト.
Actor enemyBulletList[256];			// 敵の弾のリスト.

// 自機のアニメーション.
const FrameAnimation::KeyFrame playerKeyFrames[] = {
	{ 0.000f, glm::vec2(0, 0), glm::vec2(64, 32) },
{ 0.125f, glm::vec2(0, 64), glm::vec2(64, 32) },
{ 0.250f, glm::vec2(0, 32), glm::vec2(64, 32) },
};
FrameAnimation::TimelinePtr tlPlayer;

// 爆発アニメーション.
const FrameAnimation::KeyFrame blastKeyFrames[] = {
	{ 0 / 60.0f, glm::vec2(416, 0), glm::vec2(32, 32) },
	{ 5 / 60.0f, glm::vec2(416, 32), glm::vec2(32, 32) },
	{ 10 / 60.0f, glm::vec2(416, 64), glm::vec2(32, 32) },
	{ 15 / 60.0f, glm::vec2(416, 96), glm::vec2(32, 32) },
	{ 20 / 60.0f, glm::vec2(416, 96), glm::vec2(32, 32) },
};
FrameAnimation::TimelinePtr tlBlast;

/*
* メイン画面用の構造体の初期設定を行う.
*
* @param scene		メイン画面用構造体のポインタ.
*
* @retval true		初期化成功.
* @retval false		初期化失敗.
*/
bool initialize(MainScene* scene) {
	sprBackground = Sprite("Res/UnknownPlanet.png");
	sprPlayer.spr = Sprite("Res/Objects.png", glm::vec3(0, 0, 0), Rect(0, 0, 64, 32));
	// 自機の体力.
	sprPlayer.health = 1;
	
	//enemyGenerationTimer = 2;

	// Animatorの設定.
	sprPlayer.spr.Animator(FrameAnimation::Animate::Create(tlPlayer));

	// 自機の当たり判定.
	sprPlayer.collisionShape = Rect(-24, -8, 48, 16);

	// Actor配列を初期化.
	initializeActorList(std::begin(enemyList), std::end(enemyList));
	initializeActorList(std::begin(enemyBulletList), std::end(enemyBulletList));
	initializeActorList(std::begin(playerBulletList), std::end(playerBulletList));
	initializeActorList(std::begin(playerLaserList), std::end(playerLaserList));
	initializeActorList(std::begin(effectList), std::end(effectList));
	initializeActorList(std::begin(itemList), std::end(itemList));
	
	if (scene->stageNo == 1) {
		score = 0;
		weaponLevel = weaponLevelMin;
		weaponType = weaponTypeWideShot;
	}
	
	// 敵配置マップを読み込む.
	enemyMap.Load("Res/EnemyMap.json");
	mapCurrentPosX = mapProcessedX = windowWidth;
	isEndOfMap = false;
	boss = nullptr;
	isStagePassed = false;

	// 音声を準備する.
	Audio::EngineRef audio = Audio::Engine::Instance();
	seBlast = audio.Prepare("Res/Audio/Blast.xwm");
	sePlayerShot = audio.Prepare("Res/Audio/PlayerShot.xwm");
	sePlayerLaser = audio.Prepare("Res/Audio/Laser.xwm");
	bgm = audio.Prepare("Res/Audio/Neolith.xwm");
	seBreak = audio.Prepare("Res/Audio/shock4.mp3");
	sePowerUp = audio.Prepare("Res/Audio/GetItem.xwm");
	// BGMをループ再生する.
	bgm->Play(Audio::Flag_Loop);
	return true;
}

/**
* プロトタイプ宣言.
*/
void processInput(GLFWEW::WindowRef);
void update(GLFWEW::WindowRef);
void render(GLFWEW::WindowRef);
void playerBulletAndEnemyContactHandler(Actor*, Actor*);
void playerLaserAndEnemyContactHandler(Actor*, Actor*);
void playerAndEnemyContactHandler(Actor*, Actor*);
void playerAndEnemyBulletContactHandler(Actor*, Actor*);
void playerAndItemContactHandler(Actor*, Actor*);
void stopPlayerLaser();
void generateObjectFromMap(float);
void updateEnemies(float);
void enemyblastAnimation(Actor*);
void playerblastAnimation(Actor*);

int randomTime(float, float);

/**
* プログラムのエントリーポイント.
*/
int main()
{
	// アプリケーションの初期化.
	GLFWEW::WindowRef window = GLFWEW::Window::Instance();
	if (!window.Initialize(windowWidth, windowHeight, windowTitle)) {
		return 1;
	}
	// 音声再生システムの初期化.
	Audio::EngineRef audio = Audio::Engine::Instance();
	if (!audio.Initialize()) {
		return 1;
	}
	if (!Texture::Initialize()) {
		return 1;
	}
	if (!renderer.Initialize(1024)) {
		return 1;
	}
	if (!fontRenderer.Initialize(1024,glm::vec2(windowWidth, windowHeight))) {
		return 1;
	}
	if (!fontRenderer.LoadFromFile("Res/Font/makinas_scrap.fnt")) {
		return 1;
	}

	random.seed(std::random_device()());			// 乱数エンジンの初期化.
	// タイムラインの作成
	tlPlayer = FrameAnimation::Timeline::Create(playerKeyFrames);	
	tlBlast = FrameAnimation::Timeline::Create(blastKeyFrames);

	initializeMapObjectDataList();

	// タイトル画面を初期化する.
	gamestate = gamestateTitle;
	initialize(&titleScene);

	// ゲームループ.
	while (!window.ShouldClose()) {
		processInput(window);
		update(window);
		render(window);
		audio.Update();
	}

	Texture::Finalize();
	audio.Destroy();
    return 0;
}

/**
* プレイヤーの入力を処理する.
*
* @param window ゲームを管理するウィンドウ.
*/
void processInput(GLFWEW::WindowRef window) {
	if (gamestate == gamestateTitle) {
		processInput(window, &titleScene);
		return;
	}
	else if (gamestate == gamestateGameOver) {
		processInput(window, &gameOverScene);
		return;
	}
	else if (gamestate == gamestateGameClear) {
		processInput(window, &gameClearScene);
		return;
	}
	window.Update();

	if (sprPlayer.health > 0) {
		// 自機の速度を設定する.
		const GamePad gamepad = window.GetGamePad();
		if (gamepad.buttons & GamePad::DPAD_UP) {
			playerVelocity.y = 1;
		}
		else if (gamepad.buttons & GamePad::DPAD_DOWN) {
			playerVelocity.y = -1;
		}
		else {
			playerVelocity.y = 0;
		}
		if (gamepad.buttons & GamePad::DPAD_RIGHT) {
			playerVelocity.x = 1;
		}
		else if (gamepad.buttons & GamePad::DPAD_LEFT) {
			playerVelocity.x = -1;
		}
		else {
			playerVelocity.x = 0;
		}
		if (playerVelocity.x || playerVelocity.y) {
			playerVelocity = glm::normalize(playerVelocity)* 400.0f;
		}

		// 武器の切り替え.
		if((weaponLevel >= laserLevel) && (gamepad.buttonDown & GamePad::B)) {
			if (weaponType == weaponTypeWideShot) {
				weaponType = weaponTypeLaser;
			}
			else {
				weaponType = weaponTypeWideShot;
			}
		}

		// 弾の発射.
		if((weaponType == weaponTypeWideShot) && (gamepad.buttonDown & GamePad::A)) {
			for (int i = 0; i < weaponLevel; ++i) {
				// 空いている弾の構造体を検索.
				Actor* bullet = findAvailableActor(std::begin(playerBulletList), std::end(playerBulletList));

				// 空いている構造体が見つかったら、それを使って弾を出現させる.
				if (bullet != nullptr) {
					bullet->spr = Sprite("Res/Objects.png",
						sprPlayer.spr.Position(), Rect(64, 0, 32, 16));

					const float angles[] = { 0,7.5f,-7.5f,15.0f,-15.0f,22.5f,-22.5f };
					const float radian = angles[i] / 180.0f * 3.14f;
					const float c = std::cos(radian);
					const float s = std::sin(radian);

					// 弾の移動(トウィーニング)
					bullet->spr.Tweener(TweenAnimation::Animate::Create(
						TweenAnimation::MoveBy::Create(
							1, glm::vec3(1200 * c, 1200 * s, 0), TweenAnimation::EasingType::Linear)));
					bullet->spr.Rotation(radian);
					// 弾の当たり判定.
					bullet->collisionShape = Rect(-8, -4, 16, 8);
					// 弾の体力.
					bullet->health = 1;
				}
			}
			sePlayerShot->Play();		// 弾の発射音を再生.
		}

		// 前方レーザーの発射.
		if ((weaponType == weaponTypeLaser) && (gamepad.buttons & GamePad::A)) {
			if (playerLaserList[0].health <= 0) {
				const glm::vec3 posFiringPoint = sprPlayer.spr.Position();
				playerLaserList[0].spr = Sprite("Res/Objects.png", posFiringPoint, Rect(96, 0, 32, 16));
				// レーザーの当たり判定.
				playerLaserList[0].collisionShape = Rect(-8, -4, 16, 8);
				// レーザーの体力.
				playerLaserList[0].health = 1;

				playerLaserList[1].spr = Sprite("Res/Objects.png", posFiringPoint, Rect(112, 0, 32, 16));				
				playerLaserList[1].collisionShape = Rect(-8, -4, 16, 8);
				playerLaserList[1].health = 1;

				playerLaserList[2].spr = Sprite("Res/Objects.png", posFiringPoint, Rect(128, 0, 32, 16));
				playerLaserList[2].collisionShape = Rect(-8, -4, 16, 8);
				playerLaserList[2].health = 1;
				sePlayerLaser->Play(Audio::Flag_Loop);		// レーザーの発射音を再生.
			}
		}
		else {
			// レーザーを停止する.
			stopPlayerLaser();
		}
		// 後方レーザーの発射.
		if ((weaponType == weaponTypeLaser) && (weaponLevel >= weaponLevelMax) && (gamepad.buttons & GamePad::A)) {
			if (playerLaserList[3].health <= 0) {
				const glm::vec3 posFiringPoint2 = -sprPlayer.spr.Position();
				playerLaserList[3].spr = Sprite("Res/Objects.png", posFiringPoint2, Rect(96, 0, 32, 16));
				// レーザーの当たり判定.
				playerLaserList[3].collisionShape = Rect(-8, -4, 16, 8);
				// レーザーの体力.
				playerLaserList[3].health = 1;

				playerLaserList[4].spr = Sprite("Res/Objects.png", posFiringPoint2, Rect(112, 0, 32, 16));
				playerLaserList[4].collisionShape = Rect(-8, -4, 16, 8);
				playerLaserList[4].health = 1;

				playerLaserList[5].spr = Sprite("Res/Objects.png", posFiringPoint2, Rect(128, 0, 32, 16));
				playerLaserList[5].collisionShape = Rect(-8, -4, 16, 8);
				playerLaserList[5].health = 1;				
			}
		}
	}
	else {
		playerVelocity = glm::vec3(0, 0, 0);
		stopPlayerLaser();
	}	
}


/**
* ゲームの状態を更新する.
*
* @param window ゲームを管理するウィンドウ.
*/
void update(GLFWEW::WindowRef window) {
	if (gamestate == gamestateTitle) {
		update(window, &titleScene);
		return;
	}
	else if (gamestate == gamestateMain) {
		
		// 自機が破壊されていたらゲームオーバー画面を表示する.
		if (sprPlayer.health <= 0) {
			const float deltaTime = window.DeltaTime();	// 前回の更新からの経過時間（秒）.			
			waitTimer -= deltaTime;
			// タイマーが0以下になるまでカウントダウン.
			if (waitTimer < 0) {				
				bgm->Stop();	// BGMを停止する.
				gamestate = gamestateGameOver;
				initialize(&gameOverScene);
				return;
			}
		}
	}
	else if (gamestate == gamestateGameOver) {
		update(window, &gameOverScene);
		return;
	}
	else if (gamestate == gamestateGameClear) {
		update(window, &gameClearScene);
		return;
	}

	// ステージクリア判定.
	if (!isStagePassed && boss && (boss->health <= 0)) {
		isStagePassed = true;
		boss = nullptr;
		waitTimer = 2.0f;
	}
	if (isStagePassed) {
		const float deltaTime = window.DeltaTime();	// 前回の更新からの経過時間（秒）.	
		waitTimer -= deltaTime;
		if (waitTimer <= 0) {
			stopPlayerLaser();
			++mainScene.stageNo;
			bgm->Stop();	// BGMを停止する.
			gamestate = gamestateGameClear;
			initialize(&gameClearScene);
			return;
		}
	}

	const float deltaTime = window.DeltaTime();	// 前回の更新からの経過時間（秒）.	

	if (sprPlayer.health > 0) {
		// 自機の移動.
		if (playerVelocity.x || playerVelocity.y) {
			glm::vec3 newPos = sprPlayer.spr.Position() + playerVelocity * deltaTime;
			// 自機の移動範囲を画面内に制限する.
			const Rect playerRect = sprPlayer.spr.Rectangle();
			if (newPos.x < -0.5f * (windowWidth - playerRect.size.x)) {
				newPos.x = -0.5f * (windowWidth - playerRect.size.x);
			}
			else if (newPos.x > 0.5f * (windowWidth - playerRect.size.x)) {
				newPos.x = 0.5f * (windowWidth - playerRect.size.x);
			}
			if (newPos.y < -0.5f * (windowHeight - playerRect.size.y)) {
				newPos.y = -0.5f * (windowHeight - playerRect.size.y);
			}
			else if (newPos.y > 0.5f * (windowHeight - playerRect.size.y)) {
				newPos.y = 0.5f * (windowHeight - playerRect.size.y);
			}
			sprPlayer.spr.Position(newPos);
		}
		sprPlayer.spr.Update(deltaTime);
	}
	else {
		stopPlayerLaser();
	}

	//// 得点に応じて自機の武器を強化する.
	//weaponLevel = weaponLevelMin + score / 2000;
	//if (weaponLevel > weaponLevelMax) {
	//	weaponLevel = weaponLevelMax;
	//}

	// 敵の出現.
	#if 1

	generateObjectFromMap(deltaTime);

	#else

	// 出現までの時間が0以下になったら敵を出現させる.
	enemyGenerationTimer -= deltaTime;
	if (enemyGenerationTimer <= 0) {
		// 空いている(破壊されている)敵構造体を検索.
		Actor* enemy = nullptr;
		for (Actor* i = std::begin(enemyList); i != std::end(enemyList); ++i) {
			if (i->health <= 0) {
				enemy = i;
				break;
			}
		}
		// 空いている構造体が見つかったら、それを使って敵を出現させる.
		if (enemy != nullptr) {
			// 「y_distribution」は-0.5f * 600 = -300,～ 0.5f * 600 = 300 の範囲の乱数を作り出す変数.
			const std::uniform_real_distribution<float> 
				y_distribution(-0.5f * windowHeight, 0.5f *  windowHeight);		

			// 09回　課題3用「x_distribution」は-0.5f * 800 + 400 = 0,～ 0.5f * 800 = 400 の範囲の乱数を作り出す変数.
			const std::uniform_real_distribution<float>
				x_distribution(-0.5f * windowWidth + 400, 0.5f *  windowWidth);

			// 後方からの敵出現位置
			/*enemy->spr = Sprite("Res/Objects.png",
				glm::vec3(0.5f * windowWidth, y_distribution(random), 0),
				Rect(480, 0, 32, 32));*/

			// 09回　課題3用出現位置
			enemy->spr = Sprite("Res/Objects.png",
				glm::vec3(x_distribution(random), 0.5f *  windowHeight, 0),
				Rect(480, 0, 32, 32));
			
			// Animatorの設定.
			enemy->spr.Animator(FrameAnimation::Animate::Create(tlEnemy));

			// 敵の移動(トウィーニング)

			// 横一直線の移動
			/*enemy->spr.Tweener(TweenAnimation::Animate::Create(
				TweenAnimation::MoveBy::Create(
					5.0f, glm::vec3(-1000, 0, 0), TweenAnimation::EasingType::Linear)));*/
			namespace TA = TweenAnimation;

			// 09回　1.1
			/*TA::SequencePtr seq = TA::Sequence::Create(2);
			seq->Add(TA::MoveBy::Create(1, glm::vec3(-200, 100, 0), TA::EasingType::Linear));
			seq->Add(TA::MoveBy::Create(1, glm::vec3(-200, -100, 0), TA::EasingType::Linear));
			enemy->spr.Tweener(TA::Animate::Create(seq));*/

			// 09回　課題1
			/*TA::SequencePtr seq = TA::Sequence::Create(4);
			seq->Add(TA::MoveBy::Create(1, glm::vec3(-100, 100, 0), TA::EasingType::Linear));			
			seq->Add(TA::MoveBy::Create(1, glm::vec3(-100, -100, 0), TA::EasingType::Linear));			
			enemy->spr.Tweener(TA::Animate::Create(seq));*/				
			
			// 09回　課題2
			/*TA::SequencePtr seq = TA::Sequence::Create(4);
			seq->Add(TA::MoveBy::Create(1, glm::vec3(-100, 0, 0), TA::EasingType::Linear));
			seq->Add(TA::MoveBy::Create(1, glm::vec3(0, 100, 0), TA::EasingType::Linear));
			seq->Add(TA::MoveBy::Create(1, glm::vec3(-100, 0, 0), TA::EasingType::Linear));
			seq->Add(TA::MoveBy::Create(1, glm::vec3(0, -100, 0), TA::EasingType::Linear));
			enemy->spr.Tweener(TA::Animate::Create(seq));*/

			// 09回　1.2
			/*TA::SequencePtr seq = TA::Sequence::Create(2);
			seq->Add(TA::MoveBy::Create(1, glm::vec3(0, 100, 0), TA::EasingType::EaseInOut,TA::Target::Y));
			seq->Add(TA::MoveBy::Create(1, glm::vec3(0, -100, 0), TA::EasingType::EaseInOut, TA::Target::Y));
			TA::ParallelizePtr par = TA::Parallelize::Create(1);
			par->Add(seq);
			par->Add(TA::MoveBy::Create(8, glm::vec3(-1000, 0, 0), TA::EasingType::Linear, TA::Target::X));
			enemy->spr.Tweener(TA::Animate::Create(par));*/

			// 09回　課題3 ジグザグ移動
			/*TA::SequencePtr seq = TA::Sequence::Create(2);
			seq->Add(TA::MoveBy::Create(4, glm::vec3(-400, -150, 0), TA::EasingType::Linear));
			seq->Add(TA::MoveBy::Create(4, glm::vec3(400, -150, 0), TA::EasingType::Linear));			
			enemy->spr.Tweener(TA::Animate::Create(seq));*/

			//  09回　課題3 x座標ランダム出現ver
			TA::SequencePtr seq = TA::Sequence::Create(3);
			seq->Add(TA::MoveBy::Create(1, glm::vec3(-400, 0, 0), TA::EasingType::EaseInOut, TA::Target::X));
			seq->Add(TA::MoveBy::Create(1, glm::vec3(400, 0, 0), TA::EasingType::EaseInOut, TA::Target::X));
			TA::ParallelizePtr par = TA::Parallelize::Create(1);
			par->Add(seq);
			par->Add(TA::MoveBy::Create(6, glm::vec3(0, -900, 0), TA::EasingType::Linear, TA::Target::Y));			
			enemy->spr.Tweener(TA::Animate::Create(par));	
			
			// 敵の当たり判定.
			enemy->collisionShape = Rect(-16, -16, 32, 32);
			// 敵の体力.
			enemy->health = 3;

			// 「time_distribution」は 0.5～4.0の範囲の乱数を作り出す変数.(課題1のため)
			const std::uniform_real_distribution<float>time_distribution(0.5f, 4.0f);	// 0.5～4.0の範囲の乱数を作る.
			// 次の敵が出現するまでの時間を設定する.
			enemyGenerationTimer = time_distribution(random);	// 0.5～4.0の範囲の乱数が欲しい
			
			// const std::normal_distribution<float>time_distribution(0.125f, 4.0f);
			// enemyGenerationTimer = randomTime(0.5, 4.0);	
		}
	}
	#endif
	// Actorの更新.
	updateEnemies(deltaTime);
	updateActorList(std::begin(enemyList), std::end(enemyList), deltaTime); 
	updateActorList(std::begin(enemyBulletList), std::end(enemyBulletList), deltaTime);
	updateActorList(std::begin(playerBulletList), std::end(playerBulletList), deltaTime);
	updateActorList(std::begin(playerLaserList), std::end(playerLaserList), deltaTime);
	updateActorList(std::begin(effectList), std::end(effectList), deltaTime);
	updateActorList(std::begin(itemList), std::end(itemList), deltaTime);
	
	// 前方レーザーの移動.
	if (playerLaserList[0].health > 0) {
		// レーザー(頭部)の移動.
		const float laserSpeed = 1600.0f;
		const glm::vec3 posFiringPoint = sprPlayer.spr.Position();
		glm::vec3 posHead = playerLaserList[2].spr.Position();
		posHead.x += laserSpeed * deltaTime;
		if (posHead.x > windowWidth * 0.5f) {
			posHead.x = windowWidth * 0.5f;
		}
		posHead.y = posFiringPoint.y;
		playerLaserList[2].spr.Position(posHead);

		// レーザー(腹部)の移動.
		const float halfHeadSize = playerLaserList[2].spr.Rectangle().size.x * 0.5f;
		const float halfTailSize = playerLaserList[0].spr.Rectangle().size.x * 0.5f;
		const float bodySize = playerLaserList[1].spr.Rectangle().size.x;
		const float bodyLength = posHead.x - posFiringPoint.x - halfHeadSize - halfTailSize;
				
		glm::vec3 posBody = playerLaserList[1].spr.Position();
		posBody.x = posFiringPoint.x + (posHead.x - posFiringPoint.x) * 0.5f;
		posBody.y = posFiringPoint.y;
		playerLaserList[1].spr.Position(posBody);
		playerLaserList[1].spr.Scale(glm::vec2(bodyLength / bodySize, 1));
		playerLaserList[1].collisionShape = Rect(-bodyLength * 0.5f, -4, bodyLength, 8);

		// レーザー(尾部)の移動.		
		playerLaserList[0].spr.Position(posFiringPoint);

		// 後方レーザーの移動.
		if (weaponLevel >= weaponLevelMax) {
			// レーザー(頭部)の移動.
			const float laserSpeed2 = -1600.0f;
			const glm::vec3 posFiringPoint = sprPlayer.spr.Position();
			glm::vec3 posHead2 = playerLaserList[5].spr.Position();
			posHead2.x += laserSpeed2 * deltaTime;
			if (posHead2.x < -windowWidth * 0.5f) {
				posHead2.x = -windowWidth * 0.5f;
			}
			posHead2.y = posFiringPoint.y;
			playerLaserList[5].spr.Position(posHead2);

			// レーザー(腹部)の移動.
			const float halfHeadSize2 = playerLaserList[5].spr.Rectangle().size.x * 0.5f;
			const float halfTailSize2 = playerLaserList[3].spr.Rectangle().size.x * 0.5f;
			const float bodySize2 = playerLaserList[4].spr.Rectangle().size.x;
			const float bodyLength2 = -posHead2.x + posFiringPoint.x + halfHeadSize2 + halfTailSize2;

			glm::vec3 posBody2 = playerLaserList[3].spr.Position();
			posBody2.x = posFiringPoint.x + (posHead2.x - posFiringPoint.x) * 0.5f;
			posBody2.y = posFiringPoint.y;
			playerLaserList[4].spr.Position(posBody2);
			playerLaserList[4].spr.Scale(glm::vec2(bodyLength2 / bodySize2, 1));
			playerLaserList[4].collisionShape = Rect(-bodyLength2 * 0.5f, -4, bodyLength2, 8);

			// レーザー(尾部)の移動.		
			playerLaserList[3].spr.Position(posFiringPoint);
		}
	}


	//// 敵の更新.(右端からランダム出現する場合の調整ver)	
	//for (Actor* i = std::begin(enemyList); i != std::end(enemyList); ++i) {
	//	glm::vec3 enepos = i->spr.Position();
	//	// 敵機の出現範囲を画面内に制限する.
	//	const Rect enemyRect = i->spr.Rectangle();
	//	if (enepos.y < -0.5f * (windowHeight - enemyRect.size.y)) {
	//		enepos.y = -0.5f * (windowHeight - enemyRect.size.y);
	//	}
	//	else if (enepos.y > 0.5f * (windowHeight - enemyRect.size.y)) {
	//		enepos.y = 0.5f * (windowHeight - enemyRect.size.y);
	//	}
	//	i->spr.Position(enepos);

	//	if (i->health > 0) {
	//		i->spr.Update(deltaTime);
	//		// 移動を終えた敵を破壊する.
	//		if (i->spr.Tweener()->IsFinished()) {
	//			i->health = 0;
	//		}
	//	}
	//}	
	if (!isStagePassed) {
		// 自機の弾と敵の衝突判定.
		detectCollision(std::begin(playerBulletList), std::end(playerBulletList),
			std::begin(enemyList), std::end(enemyList), playerBulletAndEnemyContactHandler);

		// 自機のレーザーと敵の衝突判定.
		detectCollision(std::begin(playerLaserList), std::end(playerLaserList),
			std::begin(enemyList), std::end(enemyList), playerLaserAndEnemyContactHandler);

		// 自機と敵の衝突判定.	
		detectCollision(&sprPlayer, &sprPlayer + 1,
			std::begin(enemyList), std::end(enemyList), playerAndEnemyContactHandler);

		// 自機と敵の弾の衝突判定.	
		detectCollision(&sprPlayer, &sprPlayer + 1,
			std::begin(enemyBulletList), std::end(enemyBulletList), playerAndEnemyBulletContactHandler);

		// 自機とアイテムの衝突判定.	
		detectCollision(&sprPlayer, &sprPlayer + 1,
			std::begin(itemList), std::end(itemList), playerAndItemContactHandler);
	}	
}

/**
* ゲームの状態を描画する.
*
* @param window ゲームを管理するウィンドウ.
*/
void render(GLFWEW::WindowRef window) {
	if (gamestate == gamestateTitle) {
		render(window, &titleScene);
		return;
	}
	else if (gamestate == gamestateGameOver) {
		render(window, &gameOverScene);
		return;
	}
	else if (gamestate == gamestateGameClear) {
		render(window, &gameClearScene);
		return;
	}

	renderer.BeginUpdate();
	renderer.AddVertices(sprBackground);
	if (sprPlayer.health > 0) {
		renderer.AddVertices(sprPlayer.spr);
	}

	// Actorの表示.
	renderActorList(std::begin(enemyList), std::end(enemyList));
	renderActorList(std::begin(enemyBulletList), std::end(enemyBulletList));
	renderActorList(std::begin(playerBulletList), std::end(playerBulletList));
	renderActorList(std::begin(playerLaserList), std::end(playerLaserList));
	renderActorList(std::begin(effectList), std::end(effectList));
	renderActorList(std::begin(itemList), std::end(itemList));

	renderer.EndUpdate();
	renderer.Draw({ windowWidth, windowHeight });

	// スコアを文字に変換して表示.
	fontRenderer.BeginUpdate();
	char str[64];
	snprintf(str, sizeof(str), "%08d", score);
	fontRenderer.AddString(glm::vec2(-100, 300), str);

	snprintf(str, sizeof(str), "STAGE: %02d", mainScene.stageNo);
	fontRenderer.Scale(glm::vec2(0.5f, 0.5f));
	fontRenderer.Color(glm::vec4(0.75f, 0.75f, 1.0f, 1.0f));
	fontRenderer.Thickness(0);
	fontRenderer.AddString(glm::vec2(-384, 300), str);
	fontRenderer.Scale(glm::vec2(1.0f, 1.0f));
	fontRenderer.Color(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	fontRenderer.Thickness(0.33f);
	
	fontRenderer.EndUpdate();
	fontRenderer.Draw();

	window.SwapBuffers();
}


/**
* 衝突を検出する.
*
* @param firstA		衝突させる配列Aの先頭ポインタ.
* @param lastA		衝突させる配列Aの終端ポインタ.
* @param firstB		衝突させる配列Bの先頭ポインタ.
* @param lastB		衝突させる配列Bの終端ポインタ.
* @param handler	A-B間で衝突が検出されたときに実行する関数.
*/
void detectCollision(Actor* firstA, Actor* lastA, Actor* firstB, Actor* lastB, CollisionHandlerType handler) {
	// aとbの衝突判定.
	for (Actor* a = firstA; a != lastA; ++a) {		
		if (a->health <= 0) {
			continue;
		}
		Rect rectA = a->collisionShape;
		rectA.origin += glm::vec2(a->spr.Position());
		for (Actor*b = firstB; b != lastB; ++b) {			
			if (b->health <= 0) {
				continue;
			}
			Rect rectB = b->collisionShape;
			rectB.origin += glm::vec2(b->spr.Position());
			if (detectCollision(&rectA, &rectB)) {
				handler(a, b);				

				if (a->health <= 0) {
					break;
				}				
			}
		}
	}
}

/**
* 自機の弾と敵の衝突を処理する.
*
* @param bullet		衝突させる配列Aの先頭ポインタ.
* @param enemy		衝突させる配列Aの終端ポインタ.
*/
void playerBulletAndEnemyContactHandler(Actor* bullet, Actor* enemy) {
	const int bulletDamage = bullet->health;
	const int enemyDamage = enemy->health;
	bullet->health -= enemyDamage;
	enemy->health -= bulletDamage;
	if (enemy->health <= 0) {
		score += enemy->score;		// 敵を破壊したら得点を増やす.
		enemyblastAnimation(enemy);
	}
}

/**
* 自機のレーザーと敵の衝突を処理する.
*
* @param laser		自機のレーザーのポインタ.
* @param enemy		敵のポインタ.
*/
void playerLaserAndEnemyContactHandler(Actor* laser, Actor* enemy) {
	
	enemy->health -= laser->health;
	if (enemy->health <= 0) {
		score += enemy->score;		// 敵を破壊したら得点を増やす.
		enemyblastAnimation(enemy);
	}
}

/**
* 自機と敵の衝突を処理する.
*
* @param player		自機のポインタ.
* @param enemy		敵のポインタ.
*/
void playerAndEnemyContactHandler(Actor* player, Actor* enemy) {
	
	if (player->health > enemy->health) {
		player->health -= enemy->health;
		enemy->health = 0;
	}
	else {
		enemy->health -= player->health;
		player->health = 0;
	}
	if (enemy->health <= 0) {
		score += enemy->score;
		enemyblastAnimation(enemy);
	}

	if (player->health <= 0) {
		playerblastAnimation(player);
		waitTimer = 2.0f;
	}
}

/**
* 自機と敵の弾の衝突を処理する.
*
* @param player		自機のポインタ.
* @param ebullet	敵の弾のポインタ.
*/
void playerAndEnemyBulletContactHandler(Actor* player, Actor* ebullet) {

	const int ebulletDamage = ebullet->health;
	const int playerDamage = player->health;
	ebullet->health -= playerDamage;
	player->health -= ebulletDamage;
	
	if (player->health <= 0) {
		playerblastAnimation(player);
		waitTimer = 2.0f;
	}
}

/**
* 自機とアイテムの衝突を処理する.
*
* @param player		自機のポインタ.
* @param item		アイテムのポインタ.
*/
void playerAndItemContactHandler(Actor* player, Actor* item) {
		
	item->health = 0;
	sePowerUp->Play();

	// 自機の武器を強化する.
	++weaponLevel;
	if (weaponLevel > weaponLevelMax) {
		weaponLevel = weaponLevelMax;
		score += item->score;
	}
}

/**
* 自機のレーザーを停止する.
*/
void stopPlayerLaser() {
	if (playerLaserList[0].health > 0) {
		for (Actor* i = std::begin(playerLaserList); i != std::end(playerLaserList); ++i) {
			i->spr = Sprite();
			i->health = 0;
		}
		sePlayerLaser->Stop();
	}
}

/**
* マップデータを見て敵やアイテムを出現させる.
*
* @param deltaTime 前回の更新からの経過時間.
*/
void generateObjectFromMap(float deltaTime) {
	if (isEndOfMap) {
		return;
	}
	const TiledMap::Layer& tiledMapLayer = enemyMap.GetLayer(0);
	const glm::vec2 tileSize = enemyMap.GetTileSet(tiledMapLayer.tilesetNo).size;
	// 敵配置マップ参照位置の更新.
	const float enemyMapScrollSpeed = 100;	// 更新速度.
	mapCurrentPosX += enemyMapScrollSpeed * deltaTime;
	if (mapCurrentPosX >= tiledMapLayer.size.x * tileSize.x) {
		// 終端を超えたら停止.
		isEndOfMap = true;
		return;
	}
	// 次の列に到達したらデータを読む.
	if (mapCurrentPosX - mapProcessedX >= tileSize.x) {
		mapProcessedX += tileSize.x;
		const int mapX = static_cast<int>(mapProcessedX / tileSize.x);
		for (int mapY = 0; mapY < tiledMapLayer.size.y; ++mapY) {
			
			const int tileId = tiledMapLayer.At(mapY, mapX);

			const MapObjectData* data = findMapObjectData(tileId);

			if (data != nullptr) {
				// 空いている(破壊されている)敵構造体の検索.
				Actor* enemy = findAvailableActor(std::begin(enemyList), std::end(enemyList));

				// 空いている構造体が見つかったら、それを使って敵を出現させる.
				if (enemy != nullptr) {
					// 後方からの敵出現位置
					const float y = (windowHeight - data->imageRect.size.y) * 0.5f - static_cast<float>(mapY * tileSize.y);
					const float x = (windowWidth + data->imageRect.size.x) * 0.5f;

					enemy->spr = Sprite("Res/Objects.png", glm::vec3(x, y, 0), data->imageRect);
					// Animatorの設定.
					enemy->spr.Animator(FrameAnimation::Animate::Create(data->tl));
					// 敵の移動(トウィーニング)					
					enemy->spr.Tweener(TweenAnimation::Animate::Create(data->createTween()));
					// 敵の当たり判定.
					enemy->collisionShape = data->collisionShape;
					// 敵の体力.
					enemy->health = data->health;
					// 敵の得点.
					enemy->score = data->score;
					// タイマーの設定.
					enemy->timer = 0;
					// IDの設定
					enemy->id = data->id;
					// ボスのポインタを設定する.
					if (data->id == tileId_Boss && boss == nullptr) {
						boss = enemy;
					}
				}
			}			
			else if (tileId == tileId_PowerUpItem) {
				// 空いているアイテム構造体の検索.
				Actor* item = findAvailableActor(std::begin(itemList), std::end(itemList));

				// 空いている構造体が見つかったら、それを使ってアイテムを出現させる.
				if (item != nullptr) {
					// 後方からのアイテム出現位置
					const float y = windowHeight * 0.5f - static_cast<float>(mapY * tileSize.x);
					item->spr = Sprite("Res/Objects.png", glm::vec3(0.5f * windowWidth, y, 0), Rect(160, 32, 32, 32));

					// アイテムの移動(トウィーニング)
					namespace TA = TweenAnimation;
					item->spr.Tweener(TA::Animate::Create(TA::MoveBy::Create(16, glm::vec3(-1000, 0, 0))));
					// アイテムの当たり判定.
					item->collisionShape = Rect(-16, -16, 32, 32);
					// アイテムの体力.
					item->health = 1;
					// アイテムの得点.
					item->score = 1000;
				}
			}

		}
	}
}

/**
* 敵の状態を更新する.
*
* @param deltaTime 前回の更新からの経過時間.
*/
void updateEnemies(float deltaTime) {
	const glm::vec3 playerPos = sprPlayer.spr.Position();
	for (Actor* i = std::begin(enemyList); i != std::end(enemyList); i++) {
		if (i->health <= 0) {
			continue;
		}

		// 一定時間ごとに弾を発射.
		const float shotInterval = 2.0f;
		i->timer += deltaTime;
		if (i->timer < shotInterval) {
			continue;
		}
		i->timer -= shotInterval;

		// 画面外にいるときは発射しない.
		const glm::vec3 enemyPos = i->spr.Position();
		if (enemyPos.x <= windowWidth * -0.5f || enemyPos.x > windowWidth * 0.5f) {
			continue;
		}
		if (enemyPos.y <= windowHeight * -0.5f || enemyPos.y > windowHeight * 0.5f) {
			continue;
		}

		// 自機の方向を計算.
		const glm::vec3 distance = playerPos - i->spr.Position();
		const float radian = std::atan2(distance.y, distance.x);
		const float c = std::cos(radian);
		const float s = std::sin(radian);

		// 小型の敵の攻撃.
		if (i->id == tileId_SmallEnemy) {
			// 空いている敵の弾を検索.
			Actor* bullet = findAvailableActor(std::begin(enemyBulletList), std::end(enemyBulletList));
			if (bullet == nullptr) {
				continue;
			}
			// 自機に向かって弾を発射.
			bullet->spr = Sprite("Res/Objects.png", i->spr.Position(), Rect(464, 0, 16, 16));
			namespace TA = TweenAnimation;
			bullet->spr.Tweener(TA::Animate::Create(TA::MoveBy::Create(8, glm::vec3(1200.0f*c, 1200.0f*s, 0))));
			bullet->spr.Rotation(radian + 3.14f);
			bullet->collisionShape = Rect(-4, -4, 8, 8);
			bullet->health = 1;
		}

		// 中型の敵の攻撃.
		if (i->id == tileId_MediumEnemy) {
			Actor* bullet = findAvailableActor(std::begin(enemyBulletList), std::end(enemyBulletList));
			if (bullet == nullptr) {
				continue;
			}
			glm::vec3 pos = i->spr.Position();
			bullet->spr = Sprite("Res/Objects.png", pos, Rect(464, 0, 16, 16));
			namespace TA = TweenAnimation;
			bullet->spr.Tweener(TA::Animate::Create(TA::MoveBy::Create(6, glm::vec3(1200.0f*c, 1200.0f*s, 0))));
			bullet->spr.Rotation(radian + 3.14f);
			bullet->spr.Scale(glm::vec2(2, 1));
			bullet->collisionShape = Rect(-8, -4, 16, 8);
			bullet->health = 1;
		}

		// 大型の敵の攻撃.
		if (i->id == tileId_LargeEnemy) {
			for (int n = 0; n < 2; ++n) {
				Actor* bullet = findAvailableActor(std::begin(enemyBulletList), std::end(enemyBulletList));
				if (bullet == nullptr) {
					continue;
				}
				glm::vec3 pos = i->spr.Position();
				pos.y += (float)(16 - n * 32);
				bullet->spr = Sprite("Res/Objects.png", pos, Rect(464, 0, 16, 16));
				namespace TA = TweenAnimation;
				bullet->spr.Tweener(TA::Animate::Create(TA::MoveBy::Create(4, glm::vec3(-1200, 0, 0))));
				bullet->spr.Scale(glm::vec2(3, 1));
				bullet->collisionShape = Rect(-12, -4, 24, 8);
				bullet->health = 1;
			}
			continue;
		}

		// ボスの攻撃.
		if (i->id == tileId_Boss) {
			for (int l = 0; l < 4; ++l) {
				Actor* bullet = findAvailableActor(std::begin(enemyBulletList), std::end(enemyBulletList));
				if (bullet == nullptr) {
					continue;
				}
				glm::vec3 pos = i->spr.Position();
				pos.y += (float)(112 - l * 32);
				// 自機に向かって弾を発射.
				bullet->spr = Sprite("Res/Objects.png", pos, Rect(448, 0, 16, 16));
				namespace TA = TweenAnimation;
				bullet->spr.Tweener(TA::Animate::Create(TA::MoveBy::Create(8, glm::vec3(1400.0f*c, 1400.0f*s, 0))));
				bullet->spr.Rotation(radian + 3.14f);
				bullet->collisionShape = Rect(-4, -4, 8, 8);
				bullet->health = 1;
			}
			for (int m = 0; m < 4; ++m) {
				Actor* bullet = findAvailableActor(std::begin(enemyBulletList), std::end(enemyBulletList));
				if (bullet == nullptr) {
					continue;
				}
				glm::vec3 pos = i->spr.Position();
				pos.y += (float)(-48 - m * 32);
				// 自機に向かって弾を発射.
				bullet->spr = Sprite("Res/Objects.png", pos, Rect(448, 0, 16, 16));
				namespace TA = TweenAnimation;
				bullet->spr.Tweener(TA::Animate::Create(TA::MoveBy::Create(8, glm::vec3(1400.0f*c, 1400.0f*s, 0))));
				bullet->spr.Rotation(radian + 3.14f);
				bullet->collisionShape = Rect(-4, -4, 8, 8);
				bullet->health = 1;
			}
			for (int n = 0; n < 2; ++n) {				
				Actor* bullet = findAvailableActor(std::begin(enemyBulletList), std::end(enemyBulletList));
				if (bullet == nullptr) {
					continue;
				}
				glm::vec3 pos = i->spr.Position();
				pos.y += (float)(16 - n * 32);
				bullet->spr = Sprite("Res/Objects.png", pos, Rect(448, 48, 32, 16));
				namespace TA = TweenAnimation;
				bullet->spr.Tweener(TA::Animate::Create(TA::MoveBy::Create(4, glm::vec3(-1200, 0, 0))));
				bullet->spr.Scale(glm::vec2(10, 2));
				bullet->collisionShape = Rect(-80, -8, 160, 16);
				bullet->health = 3;
			}
			continue;
		}
	}
}

/**
* enemy爆発アニメーション関数.
*/
void enemyblastAnimation(Actor* enemy) {
	Actor* blast = findAvailableActor(std::begin(effectList), std::end(effectList));
	if (blast != nullptr) {
		blast->spr = Sprite("Res/Objects.png", enemy->spr.Position());
		blast->spr.Animator(FrameAnimation::Animate::Create(tlBlast));
		namespace TA = TweenAnimation;
		blast->spr.Tweener(TA::Animate::Create(
			TA::Rotation::Create(20 / 60.0f, glm::pi<float>() * 0.5f)));
		blast->health = 1;
		seBlast->Play();		// 爆発音を再生.
	}
}

/**
* player爆発アニメーション関数.
*/
void playerblastAnimation(Actor* player) {
	Actor* blast = findAvailableActor(std::begin(effectList), std::end(effectList));
	if (blast != nullptr) {
		blast->spr = Sprite("Res/Objects.png", player->spr.Position());
		blast->spr.Animator(FrameAnimation::Animate::Create(tlBlast));
		namespace TA = TweenAnimation;
		blast->spr.Tweener(TA::Animate::Create(
			TA::Rotation::Create(20 / 60.0f, glm::pi<float>() * 0.5f)));
		blast->spr.Scale(glm::vec2(2, 2));			// 自機の爆発は少し大きめに
		blast->health = 1;
		seBreak->Play();		// 爆発音を再生.
	}
}

// 設定した最小値と最大値から乱数を得るrandomTime関数.
int randomTime(float min, float max)
{
	return std::uniform_real_distribution<float>(min, max)(random);
}


