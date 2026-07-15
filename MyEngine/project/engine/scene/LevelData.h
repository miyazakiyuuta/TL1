#pragma once
#include "math/Vector3.h"

#include <string>
#include <vector>

// Blenderのレベルエディタ(level_editor.py)が出力したレベルデータ1シーン分。
// JSONから読み込んだ汎用的なツリーのままでは使いにくいため、
// ゲームで使う情報だけをこの入れ物に整理して格納する。
struct LevelData {
	// コライダー1個分のパラメータ
	struct ColliderData {
		std::string type;                      // 形状の種別("BOX"など)
		Vector3 center = { 0.0f, 0.0f, 0.0f }; // オブジェクト中心からのオフセット(ゲームの座標系に変換済み)
		Vector3 size = { 1.0f, 1.0f, 1.0f };   // 各軸の大きさ(ゲームの座標系に変換済み)
	};

	// 配置オブジェクト1個分のデータ
	struct ObjectData {
		std::string name;                           // Blender上のオブジェクト名
		std::string fileName;                       // モデルファイル名(resourcesからの相対パス。未指定なら空)
		Vector3 translation = { 0.0f, 0.0f, 0.0f }; // 平行移動(ゲームの座標系に変換済み)
		Vector3 rotation = { 0.0f, 0.0f, 0.0f };    // 回転角(ラジアン。ゲームの座標系に変換済み)
		Vector3 scaling = { 1.0f, 1.0f, 1.0f };     // スケーリング(ゲームの座標系に変換済み)
		bool hasCollider = false;                   // コライダーが設定されているか
		ColliderData collider;                      // hasColliderがtrueのときのみ有効
	};

	// 配置オブジェクトのリスト(親子階層は平坦化済み)
	std::vector<ObjectData> objects;
};
