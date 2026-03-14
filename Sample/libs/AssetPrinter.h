#pragma once
#include <Siv3D.hpp>
// AssetPrinter クラス（デバッグ表示用Addon）
class AssetPrinter : public IAddon
{
public:
	struct AssetLoadInfo {
		double loadTimeMs = 0.0;  // ミリ秒単位のロード時間
	};

	// TextureAsset::Register をラップし、ロード時間を計測する関数
	static bool RegisterTexture(const String& name, const String& path)
	{
		auto start = std::chrono::steady_clock::now();
		bool success = TextureAsset::Register(name, path);
		auto end = std::chrono::steady_clock::now();
		double durationMs = std::chrono::duration<double, std::milli>(end - start).count();
		if (success)
		{
			assetLoadInfos[name] = { durationMs };
		}
		return success;
	}

	static void Condition()
	{
#ifdef _DEBUG
		if (KeyF3.down())
		{
			const bool isActive = AssetPrinter::IsActive();
			if (isActive)
			{
				AssetPrinter::SetNotActive();
			}
			else
			{
				AssetPrinter::SetActive();
			}
		}
#endif

		if (AssetPrinter::IsActive())
		{
			double hh = Mouse::Wheel();
			if (hh != 0.0)
				AssetPrinter::Scr(hh);
		}
	}

	[[nodiscard]]
	static bool IsActive()
	{
		if (auto p = Addon::GetAddon<AssetPrinter>(U"AssetPrinter"))
		{
			return p->m_active;
		}
		else
		{
			return false;
		}
	}

	static void SetActive()
	{
		if (auto p = Addon::GetAddon<AssetPrinter>(U"AssetPrinter"))
		{
			p->m_active = true;
		}
	}

	static void SetNotActive()
	{
		if (auto p = Addon::GetAddon<AssetPrinter>(U"AssetPrinter"))
		{
			p->m_active = false;
		}
	}

	static void Scr(double hh)
	{
		if (auto p = Addon::GetAddon<AssetPrinter>(U"AssetPrinter"))
		{
			p->scr(hh);
		}
	}

	static void AssetUpdate()
	{
		if (auto p = Addon::GetAddon<AssetPrinter>(U"AssetPrinter"))
		{
			p->assetUpdate();
		}
	}
	static void RegisterFalse(const String& name)
	{
		if (auto p = Addon::GetAddon<AssetPrinter>(U"AssetPrinter"))
		{
			p->registerFalse(name);
		}
	}

private:
	// アセット名とそのロード情報を記録するマップ
	inline static std::unordered_map<String, AssetLoadInfo> assetLoadInfos;

	void scr(double hh)
	{
		scrollY -= hh * 10;
		scrUpdate();
	}
	void scrUpdate()
	{
		// 各アセットの元のY座標にスクロールオフセットを加える
		for (auto& ttt : m_myStructs)
		{
			ttt.rectName.y = ttt.initialY + scrollY;
			ttt.rectExecuteLoad.y = ttt.initialY + scrollY;
		}
	}
	void assetUpdate()
	{
		// 登録済みアセット数に変化があれば再初期化
		if (te != TextureAsset::Enumerate().size())
		{
			init();
		}
	}

	Array<String> m_assetNoUsage;
	void registerFalse(const String& name)
	{
		m_assetNoUsage.push_back(name);
	}

	long te = 0;
	bool init() override
	{
#ifdef _DEBUG
		m_myStructs.clear();
		int32 counter = 0;
		for (auto&& [name, info] : TextureAsset::Enumerate())
		{
			// AssetPrinter からロード時間を取得
			double loadTime = 0.0;
			if (assetLoadInfos.contains(name))
			{
				loadTime = assetLoadInfos[name].loadTimeMs;
			}

			int32 posY = counter * font.height();
			Rect nameRect{ 0, posY, 320, font.height() };
			Rect btnLoadRect{ 320, posY, 120, font.height() };
			m_myStructs.push_back({ nameRect, btnLoadRect, name, U"Load", loadTime, posY });
			counter++;
		}
		te = TextureAsset::Enumerate().size();
		scrUpdate();
#endif
		return true;
	}

	bool update() override
	{
		if (!m_active)
			return true;

		// 各アセットごとにロードボタンのクリック処理
		for (auto& ttt : m_myStructs)
		{
			if (ttt.rectExecuteLoad.leftClicked())
			{
				if (!TextureAsset::IsReady(ttt.name))
				{
					TextureAsset::Load(ttt.name);
				}
			}
		}

		return true;
	}

	void draw() const override
	{
		if (!m_active)
			return;

		// 各アセットの情報（アセット名＋ロード時間）を描画
		for (const auto& ttt : m_myStructs)
		{
			// 表示範囲内なら描画
			if (ttt.initialY + scrollY <= Scene::Size().y)
			{
				ttt.rectName.draw(Palette::White).drawFrame(1.0, Palette::Black);
				// アセット名とロード時間（小数点2桁）の描画
				font(ttt.name + U" - " + U"{:.3f}"_fmt(ttt.loadTime)).draw(ttt.rectName.x, ttt.rectName.y, Palette::Black);
				ttt.rectExecuteLoad.draw(Palette::White).drawFrame(1.0, Palette::Black);
				font(ttt.executeLoad).draw(ttt.rectExecuteLoad.x, ttt.rectExecuteLoad.y, Palette::Black);

				if (TextureAsset::IsReady(ttt.name))
				{
					TextureAsset(ttt.name).resized(64).draw(ttt.rectExecuteLoad.x + ttt.rectExecuteLoad.w, ttt.rectExecuteLoad.y);
				}
			}
		}
		// 登録失敗したアセットの表示（あれば）
		int32 counter = m_myStructs.size();
		for (const auto& name : m_assetNoUsage)
		{
			Rect{ 0, counter * font.height() + scrollY, 320, font.height() }.draw(Palette::Darkred).drawFrame(1.0, Palette::Aliceblue);
			font(name).draw(0, counter * font.height() + scrollY, Palette::Aliceblue);
			counter++;
		}
	}

	bool m_active = false;
	Font font{ 32 };
	int32 scrollY = 0;

	// 表示する各アセットの情報構造体
	struct MyStruct
	{
		Rect rectName;
		Rect rectExecuteLoad;
		String name;
		String executeLoad;
		double loadTime;  // ロード時間（ミリ秒）
		int32 initialY;   // 初期のY座標（スクロール補正用）
	};
	Array<MyStruct> m_myStructs;
};
