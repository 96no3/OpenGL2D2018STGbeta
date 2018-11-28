/**
* @file Actor.cpp
*/
#include "Actor.h"
#include "GameData.h"

/**
* 2つの長方形の状態を調べる.
*
* @param lhs 長方形その1.
* @param rhs 長方形その2.
*
* @retval true	衝突している.
* @retval false	衝突していない.
*/
bool detectCollision(const Rect* lhs, const Rect* rhs) {
	return
		lhs->origin.x < rhs->origin.x + rhs->size.x &&
		lhs->origin.x + lhs->size.x > rhs->origin.x &&
		lhs->origin.y < rhs->origin.y + rhs->size.y &&
		lhs->origin.y + lhs->size.y > rhs->origin.y;
}

/**
* Actorの配列を初期化する.
*
* @param first	初期化する配列の先頭.
* @param last	初期化する配列の終端.
*/

void initializeActorList(Actor* first, Actor* last) {

	for (Actor* i = first; i != last; ++i) {
		i->health = 0;
	}
}

/**
* Actorの配列を更新する.
*
* @param first		更新する配列の先頭.
* @param last		更新する配列の終端.
* @param deltaTime	前回の更新からの経過時間.
*/

void updateActorList(Actor* first, Actor* last, float deltaTime) {
	for (Actor* i = first; i != last; ++i) {
		if (i->health > 0) {
			i->spr.Update(deltaTime);
			// 移動を終えた弾を破壊する.
			if (i->spr.Tweener() && i->spr.Tweener()->IsFinished()) {
				i->health = 0;
			}
		}
	}
}

/**
* Actorの配列を描画する.
*
* @param first		描画する配列の先頭.
* @param last		描画する配列の終端.
*/

void renderActorList(Actor* first, Actor* last) {
	for (const Actor* i = first; i != last; ++i) {
		if (i->health > 0) {
			renderer.AddVertices(i->spr);
		}
	}
}

/**
* 利用可能なActorを取得する.
*
* @param first		検索対象の先頭要素のポインタ.
* @param last		検索対象の終端要素のポインタ.
*
* @return			利用可能なActorのポインタ.
*					利用可能なActorが見つからなければnullptr.
*
* [first, last]の範囲から、利用可能な(healthが0以下の)Actorを検索する.
*/
Actor* findAvailableActor(Actor* first, Actor* last) {
	Actor* result = nullptr;
	for (Actor* i = first; i != last; ++i) {
		if (i->health <= 0) {
			result = i;
			break;
		}
	}
	return result;
}