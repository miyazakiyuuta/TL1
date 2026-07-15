#include "scene/LevelDataLoader.h"

#include "utility/Logger.h"

#include <nlohmann/json.hpp>

#include <cassert>
#include <fstream>
#include <numbers>

namespace {
	// レベルデータの置き場所(作業ディレクトリ=プロジェクト直下からの相対パス)
	const std::string kDefaultBaseDirectory = "resources/scenes/";
	// レベルデータファイルの拡張子
	const std::string kExtension = ".json";

	// 度数法 → ラジアン(Blenderのエクスポーターは度数法で出力している)
	float ToRadian(float degree) {
		return degree * std::numbers::pi_v<float> / 180.0f;
	}

	// "objects"配列を走査してlevelDataへ詰め替える(childrenは再帰呼び出しで辿る)
	// 親子階層は平坦化して読むため、子のトランスフォームは親のローカル座標のままになる
	void LoadObjectsRecursive(const nlohmann::json& objects, LevelData& levelData) {
		for (const nlohmann::json& object : objects) {
			// 各オブジェクトには必ず"type"を入れているので、無ければ不正なデータ
			assert(object.contains("type"));

			// 種別を取得
			const std::string type = object["type"].get<std::string>();

			// MESH
			if (type.compare("MESH") == 0) {
				// 要素追加
				levelData.objects.emplace_back(LevelData::ObjectData{});
				// 今追加した要素の参照を得る
				LevelData::ObjectData& objectData = levelData.objects.back();

				// オブジェクト名
				if (object.contains("name")) {
					objectData.name = object["name"];
				}
				// ファイル名
				if (object.contains("file_name")) {
					objectData.fileName = object["file_name"];
				}

				// トランスフォームのパラメータ読み込み
				const nlohmann::json& transform = object["transform"];
				// 平行移動: Blenderは右手系Z-up、ゲームは左手系Y-upなのでYとZを入れ替える
				objectData.translation.x = (float)transform["translation"][0];
				objectData.translation.y = (float)transform["translation"][2];
				objectData.translation.z = (float)transform["translation"][1];
				// 回転角: 軸の入れ替えに加えて回転の正方向が逆になるため符号を反転する。
				// エクスポーターは度数法で出力しているのでラジアンへ変換する
				objectData.rotation.x = -ToRadian((float)transform["rotation"][0]);
				objectData.rotation.y = -ToRadian((float)transform["rotation"][2]);
				objectData.rotation.z = -ToRadian((float)transform["rotation"][1]);
				// スケーリング
				objectData.scaling.x = (float)transform["scaling"][0];
				objectData.scaling.y = (float)transform["scaling"][2];
				objectData.scaling.z = (float)transform["scaling"][1];

				// コライダーのパラメータ読み込み(設定されているオブジェクトのみ)
				if (object.contains("collider")) {
					const nlohmann::json& collider = object["collider"];
					objectData.hasCollider = true;
					objectData.collider.type = collider["type"];
					// 平行移動と同様にYとZを入れ替える
					objectData.collider.center.x = (float)collider["center"][0];
					objectData.collider.center.y = (float)collider["center"][2];
					objectData.collider.center.z = (float)collider["center"][1];
					objectData.collider.size.x = (float)collider["size"][0];
					objectData.collider.size.y = (float)collider["size"][2];
					objectData.collider.size.z = (float)collider["size"][1];
				}
			}

			// 子ノードがあれば再帰的に走査する
			if (object.contains("children")) {
				LoadObjectsRecursive(object["children"], levelData);
			}
		}
	}
}

namespace LevelDataLoader {

	std::unique_ptr<LevelData> Load(const std::string& fileName) {
		// 連結してフルパスを得る
		const std::string fullpath = kDefaultBaseDirectory + fileName + kExtension;

		// ファイルストリーム
		std::ifstream file;
		// ファイルを開く
		file.open(fullpath);
		// ファイルオープン失敗をチェック
		if (file.fail()) {
			Logger::Log("[LevelDataLoader] Failed to open: " + fullpath + "\n");
			assert(0);
			return nullptr;
		}

		// JSON文字列から解凍したデータ
		nlohmann::json deserialized;
		// 解凍
		file >> deserialized;

		// 正しいレベルデータファイルかチェック
		assert(deserialized.is_object());
		assert(deserialized.contains("name"));
		assert(deserialized["name"].is_string());

		// "name"を文字列として取得
		const std::string name = deserialized["name"].get<std::string>();
		// 出力ファイルの先頭は"scene"と決めているので、違えば不正なレベルデータ
		assert(name.compare("scene") == 0);

		// レベルデータ格納用インスタンスを生成
		std::unique_ptr<LevelData> levelData = std::make_unique<LevelData>();

		// "objects"の全オブジェクトを走査(childrenも再帰で辿る)
		LoadObjectsRecursive(deserialized["objects"], *levelData);

		Logger::Log("[LevelDataLoader] Loaded " + fullpath + " (" + std::to_string(levelData->objects.size()) + " objects)\n");

		return levelData;
	}
}
