#pragma once
#include "scene/LevelData.h"

#include <memory>
#include <string>

// Blenderのレベルエディタが出力したJSONレベルデータを読み込むローダー。
// Blender(右手系Z-up)とゲーム(左手系Y-up)の座標系の違いはここで吸収する。
namespace LevelDataLoader {
	/// <summary>
	/// レベルデータファイルを読み込む
	/// </summary>
	/// <param name="fileName">拡張子抜きのファイル名。"resources/scenes/<fileName>.json" が読まれる</param>
	/// <returns>整理済みのレベルデータ(ファイルが開けない場合はnullptr)</returns>
	std::unique_ptr<LevelData> Load(const std::string& fileName);
}
