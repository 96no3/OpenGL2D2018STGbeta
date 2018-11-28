#pragma once
/**
* @file Actor.h
*
* Actor	構造体およびActor構造体に関連する関数を宣言するヘッダファイル.
*/
#include "Sprite.h"

/**
* ゲームキャラクターの構造体.
*/
struct Actor {
	Sprite spr;				// 画像表示用スプライト.
	Rect collisionShape;	// 衝突判定の位置と大きさ.
	int health;				// 耐久力(0以下なら破壊されている).
	int type;				// 種類.
	int score;				// 得点.
	float timer;			// 多目的タイマー.
	int id;					// 識別用ID.
};
bool detectCollision(const Rect*, const Rect*);
void initializeActorList(Actor*, Actor*);
void updateActorList(Actor*, Actor*, float);
void renderActorList(Actor*, Actor*);
Actor* findAvailableActor(Actor*, Actor*);
using CollisionHandlerType = void(*)(Actor*, Actor*);	// 衝突処理関数の型.
void detectCollision(Actor*, Actor*, Actor*, Actor*, CollisionHandlerType);