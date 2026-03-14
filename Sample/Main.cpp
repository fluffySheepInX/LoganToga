# include <Siv3D.hpp> // Siv3D v0.6.16
# include "libs/AddonGaussian.h"
# include "libs/AddonSave.h"
# include "libs/AddonEffFs.h"
# include "libs/MultiTextureMaskRenderer.h"
# include "libs/AnimationEffect.hpp"
# include "libs/AssetPrinter.h"
# include "libs/Tab.hpp"
# include "libs/BeatVisualizerAddon.hpp"
# include "libs/ButtonAddon.hpp"
# include "libs/LabelAddon.hpp"
import MemoryMonitor;

// NinePatch インスタンスを作るための汎用的なファクトリー関数
// - `sourcePath` を指定すれば外部画像を読み込んで利用できる
// - `sourcePath` が未指定または読めない場合はプレースホルダー画像を生成する
// - `patch` や `theme` を差し替えれば将来的なデザイン変更にも対応しやすい
NinePatch CreateNinePatch(const Optional<FilePath>& sourcePath = none, const int32 patch = 64)
{
	if (sourcePath && FileSystem::Exists(*sourcePath))
	{
		// TODO: 外部アセットを管理する仕組みがあればここで差し替える
		const Texture texture{ *sourcePath };
		return NinePatch{ texture, patch };
	}

	// 再利用しやすい色設定をまとめて保持する構造体
	struct NinePatchTheme
	{
		ColorF corner{ 0.80, 0.80, 0.85 }; // 四隅用
		ColorF edge{ 0.86, 0.86, 0.90 };   // エッジ用
		ColorF center{ 0.94, 0.94, 0.96 }; // 中央用
		Color frame{ 30, 30, 30 };         // 外枠
	};

	const NinePatchTheme theme;

	// ------ プレースホルダー生成ブロック（素材がない場合のデフォルト） ------
	const Size srcSize{ patch * 3, patch * 3 };
	RenderTexture placeholder{ srcSize, theme.center };
	{
		// 3x3 タイルを描画してシンプルな NinePatch 素材を構築
		ScopedRenderTarget2D rt{ placeholder };
		placeholder.clear(theme.center);

		Rect{ 0, 0, patch, patch }.draw(theme.corner);
		Rect{ patch, 0, patch, patch }.draw(theme.edge);
		Rect{ (patch * 2), 0, patch, patch }.draw(theme.corner);

		Rect{ 0, patch, patch, patch }.draw(theme.edge);
		Rect{ patch, patch, patch, patch }.draw(theme.center);
		Rect{ (patch * 2), patch, patch, patch }.draw(theme.edge);

		Rect{ 0, (patch * 2), patch, patch }.draw(theme.corner);
		Rect{ patch, (patch * 2), patch, patch }.draw(theme.edge);
		Rect{ (patch * 2), (patch * 2), patch, patch }.draw(theme.corner);

		// 輪郭線を描いて視認性を確保
		Rect{ 0, 0, srcSize }.drawFrame(4, 0, theme.frame);
	}
	// -------------------------------------------------------------------

	return NinePatch{ placeholder, patch };
}

struct ClickState
{
	// クリックの履歴と集計を一箇所に集約し、UI側からの通知だけで状態を更新する
	// （再生やログ出力もここに集めることで「何をしたいか」が明確になる）

	// 押下されたボタンIDの時系列。履歴再生の入力元になる
	Array<int32> historyId;
	// これまでの総クリック数。画面表示とログの共通ソース
	int32 totalClicks = 0;
	// 最後にクリックされたID。直近の状態表示に使う
	int32 lastClickedId = -1;

	// ボタンがクリックされた事実だけを受け取り、履歴・集計・表示用状態を更新する
	void OnClicked(int32 id)
	{
		historyId.push_back(id);
		++totalClicks;
		lastClickedId = id;
		Print << U"クリック数: " + Format(totalClicks) + U" / 最後のID: " + Format(lastClickedId);
	}

	// 履歴に基づいてクリックを再生し、再生後は履歴をリセットする
	void Replay()
	{
		Print << U"履歴を再生";

		for (const auto id : historyId)
		{
			ButtonAddon::RequestCommand(id);
		}
		historyId.clear();
	}
};

void Main()
{
#pragma region Addon
	Addon::Register<GaussianFSAddon>(U"GaussianFSAddon");
	GaussianFSAddon::Condition({ 1600,900 });
	GaussianFSAddon::SetLangSet({
		{ U"Japan",     U"日本語" },
		{ U"English",   U"English" },
		{ U"Deutsch",   U"Deutsch" },
		{ U"Test",      U"TestLang" },
		});
	GaussianFSAddon::SetLang(U"Japan");
	GaussianFSAddon::SetSceneSet({
		{ U"1600*900", U"1600",U"900"},
		{ U"1200*600", U"1200",U"600"},
		});
	GaussianFSAddon::SetScene(U"1600*900");
	//GaussianFSAddon::SetSceneName(U"SelectLang");

	Addon::Register<SaveFSAddon>(U"SaveFSAddon");
	SaveFSAddon::Condition({ U"lang" ,U"Test" });

	Addon::Register<AddonEffFs>(U"AddonEffFs");
	AddonEffFs::SetKind(AddonEffFs::EffectKind::Wind);
	AddonEffFs::SetColor(ColorF(0.9, 0.95, 1.0, 0.8));
	AddonEffFs::SetSpawnDensity(0.05, 0.25);
	//AddonEffFs::SetDirection(Vec2{ 0.5, 1.0 });       // 斜めの豪雨
	//AddonEffFs::AddImpulse(1.0, 5.0);                 // 強雨タイム

	//// デフォルト設定で NinePatch を生成（必要になれば引数で差し替え可能）
	//NinePatch ninePatch = CreateNinePatch();

#pragma region マスク
	//const Texture emoji{ U"example/test.png", TextureDesc::Mipped };
	//const Texture emoji2{ U"example/test2.png", TextureDesc::Mipped };
	//const Texture windmill{ U"example/windmill.png", TextureDesc::Mipped };

	//Addon::Register(MultiTextureMaskRenderer::AddonName,
	//	std::make_unique<MultiTextureMaskRenderer>(
	//		Size{ 480, 320 },
	//		U"example/shader/hlsl/multi_texture_mask.hlsl",
	//		U"example/shader/glsl/multi_texture_mask.frag"));

	//auto* maskRenderer = Addon::GetAddon<MultiTextureMaskRenderer>(MultiTextureMaskRenderer::AddonName);
	//if (not maskRenderer || (not maskRenderer->IsValid()))
	//{
	//	throw Error{ U"Failed to load a shader file" };
	//}
#pragma endregion

#pragma region パーティクル
	//const Texture particleTexture{ U"example/particle.png", TextureDesc::Mipped };

	//ParticleSystemAddon::RegisterDefault();

	//using Kind = ParticleSystemAddon::Kind;

	//ParticleSystemAddon::Preset circlePreset;
	//circlePreset.texture = particleTexture;
	//circlePreset.parameters.rate = 280.0;
	//circlePreset.parameters.maxParticles = 1200.0;
	//circlePreset.parameters.startLifeTime = 1.2;
	//circlePreset.parameters.startSpeed = 160.0;
	//circlePreset.parameters.startSize = 28.0;
	//circlePreset.circleEmitter.sourceRadius = 6.0;
	//circlePreset.circleEmitter.r = 48.0;
	//circlePreset.circleEmitter.randomDirection = true;
	//circlePreset.durationSec = 0.25;
	//ParticleSystemAddon::SetPreset(Kind::Circle, std::move(circlePreset));
#pragma endregion

#pragma region animation
	///// 各矩形ごとに、元の矩形情報とそのアニメーション処理を持たせる
	//struct AnimatedRect {
	//	RectF baseRect;         // 元となる矩形
	//	AnimationEffect<RectF> clickAnimation; // クリック用アニメーション
	//	AnimationEffect<RectF> hoverAnimation; // ホバー用アニメーション

	//	AnimatedRect(const RectF& rect)
	//		: baseRect(rect)
	//		, clickAnimation([](RectF rect, double t) {
	//		ColorF color{ 0.0 };

	//		// 四角形の拡大アニメーション（EaseOutBack で動きを付ける）
	//		if (0.0 <= t && t < 1.0)
	//		{
	//			rect = rect.scaled(EaseOutBack(t));
	//		}

	//		// 色の変化：t の値に応じて黒→白、または白→黒に変化
	//		if (0.58 <= t && t < 1.0)
	//		{
	//			color = ColorF((t - 0.58) / 0.42);  // 黒 → 白
	//		}
	//		else if (1.0 <= t && t < 1.8)
	//		{
	//			color = ColorF((1.8 - t) / 0.8);    // 白 → 黒
	//		}

	//		rect.draw(color);
	//		})
	//		, hoverAnimation([](RectF rect, double t) {
	//		const double pulse = 0.02 * (1.0 + Sin(t * 6.0));
	//		rect = rect.scaled(1.0 + pulse);
	//		rect.draw(ColorF(0.85));
	//		})
	//	{
	//	}
	//};
	//// 各矩形とそのアニメーション処理をまとめた配列を用意
	//int32 count = 1;
	//Array<AnimatedRect> animatedRects;
	//for (size_t i = 0; i < count; i++)
	//{
	//	animatedRects.emplace_back(RectF(32 + 192, i * 30 + 32 + 192, 192));
	//}
#pragma endregion

	//Addon::Register<AssetPrinter>(U"AssetPrinter");

	//// ロード時間計測付きのテクスチャ登録
	//AssetPrinter::RegisterTexture(U"Windmill", U"example/windmill.png")
	//	? 0 : (AssetPrinter::RegisterFalse(U"Windmill"), 1);
	//AssetPrinter::RegisterTexture(U"Windmillaaaa", U"example/Windmillaaaa.png")
	//	? 0 : (AssetPrinter::RegisterFalse(U"Windmillaaaa"), 1);

#pragma region tab
	//const Array<String> items = { U"ステータス", U"武器", U"装備", U"スキル", U"任務", U"プロフィール" };
	//constexpr ColorF TabColor{ 0.2, 0.5, 0.9 };
	//constexpr ColorF TabOutlineColor{ 0.5 };
	//constexpr ColorF ContentColor{ 0.5 };

	//TabA tabA{ Size{ 160, 50 }, items };
	//TabB tabB{ Size{ 160, 50 }, items };
	//TabC tabC{ Size{ 182, 50 }, items };
#pragma endregion

	//// 音声ファイルの読み込み
	//Audio audio(U"aaa.mp3");
	//audio.play();

	//BeatVisualizerAddon::Register();

	//ButtonAddon::Register();
	//LabelAddon::Register();

	//ButtonAddon::ClearButtons();
	//LabelAddon::ClearLabels();
	//ButtonAddon::SetOnCommand([&](ButtonAddon::Command id)
	//	{
	//	m_state.OnClicked(id);
	//	});
	//
	//ButtonAddon::AddButton({ .rect = Rect{0, 0, 240, 120}, .text = U"", .font = m_buttonFont, .command = 0 });
	//ButtonAddon::AddButton({ .rect = Rect{0, 140, 240, 120}, .text = U"", .font = m_buttonFont, .command = 5 });
	//ButtonAddon::AddButton({ .rect = Rect{0, 280, 240, 120}, .text = U"", .font = m_buttonFont, .command = 10 });
	//
	//m_totalLabel = LabelAddon::AddLabel({ .text = U"合計クリック数: 0", .pos = Vec2{ 0, 360 }, .font = m_labelFont, .color = ColorF{ 0.1, 0.1, 0.1 } });
	//m_lastLabel = LabelAddon::AddLabel({ .text = U"最後のID: -1", .pos = Vec2{ 0, 380 }, .font = m_labelFont, .color = ColorF{ 0.1, 0.1, 0.1 } });
	//m_titleLabel = LabelAddon::AddLabel(m_headerLayout.MakeSpec(U"ステータス", 0, m_titleFont, ColorF{ 0.9, 0.92, 0.98 }));
	//m_pageLabel = LabelAddon::AddLabel(m_headerLayout.MakeSpec(U"ページ: 1", 1));
	//m_hpLabel = LabelAddon::AddLabel(m_statLayout.MakeSpec(U"", 0, m_textFont, ColorF{ 0.9, 0.85, 0.9 }));
	//m_mpLabel = LabelAddon::AddLabel(m_statLayout.MakeSpec(U"", 1, m_textFont, ColorF{ 0.85, 0.9, 0.95 }));
	//UpdateLabels();
	//m_statLayout.Update(m_hpLabel, U"HP: " + Format(m_state.hp), 0, m_textFont, ColorF{ 0.9, 0.85, 0.9 }, 0, false, true);
	//m_statLayout.Update(m_mpLabel, U"MP: " + Format(m_state.mp), 1, m_textFont, ColorF{ 0.85, 0.9, 0.95 }, 0, false, true);
	//m_statLayout.Update(m_atkLabel, U"攻撃: " + Format(m_state.atk), 2, m_textFont, ColorF{ 0.9, 0.9, 0.8 }, 0, false, true);
	//m_statLayout.Update(m_defLabel, U"防御: " + Format(m_state.def), 3, m_textFont, ColorF{ 0.9, 0.9, 0.8 }, 0, false, true);
	//m_statLayout.Update(m_critLabel, U"", 0, m_textFont, ColorF{ 0.9, 0.9, 0.8 }, 0, false, false);
	//m_statLayout.Update(m_evasionLabel, U"", 1, m_textFont, ColorF{ 0.9, 0.9, 0.8 }, 0, false, false);
	//m_statLayout.Update(m_speedLabel, U"", 2, m_textFont, ColorF{ 0.9, 0.9, 0.8 }, 0, false, false);
	//LabelAddon::VerticalStyle m_headerStyle{ .origin = Vec2{ 30, 80 }, .lineHeight = 30.0, .font = m_textFont, .color = ColorF{ 0.7, 0.75, 0.9 } };
	//LabelAddon::VerticalStyle m_statStyle{ .origin = Vec2{ 30, 140 }, .lineHeight = 25.0, .font = m_textFont, .color = ColorF{ 0.9, 0.9, 0.8 } });
	//LabelAddon::VerticalStyle m_detailStyle{ .origin = Vec2{ 30, 245 }, .lineHeight = 20.0, .font = m_detailFont, .color = ColorF{ 0.7, 0.75, 0.85 } };
	//LabelAddon::VerticalLayout m_headerLayout = LabelAddon::VerticalLayout::FromStyle(m_headerStyle);
	//LabelAddon::VerticalLayout m_statLayout = LabelAddon::VerticalLayout::FromStyle(m_statStyle);
	//LabelAddon::VerticalLayout m_detailLayout = LabelAddon::VerticalLayout::FromStyle(m_detailStyle);
	//LabelAddon::LabelHandle m_titleLabel = 0;
#pragma endregion

#pragma region コマンドライン
	int32 argc = System::GetArgc();
	char** argv = System::GetArgv();
	// 0:パス
	// 1:コマンドライン引数一つ目
	Array<String> args;
	for (size_t i = 0; i < argc; i++)
		args.push_back(Unicode::Widen(argv[i]));
#pragma endregion

	const Font uiFont{ 18 };
	double windAngle = 0.0;
	AddonEffFs::EffectKind currentKind = AddonEffFs::EffectKind::Wind;

	while (System::Update())
	{
#pragma region Addon
		if (GaussianFSAddon::TriggerOrDisplayESC()) break;
		if (GaussianFSAddon::TriggerOrDisplayLang()) break;
		if (GaussianFSAddon::TriggerOrDisplaySceneSize()) break;
		if (GaussianFSAddon::IsHide()) Window::Minimize();
		if (GaussianFSAddon::IsGameEnd()) break;
		GaussianFSAddon::DragProcessWindow();
		AddonEffFs::SetDeltaTime(Scene::DeltaTime());

		MemoryMonitor::Draw();
#pragma endregion

#pragma region demo
		if (Key1.down())
		{
			currentKind = AddonEffFs::EffectKind::Wind;
			AddonEffFs::SetKind(currentKind);
			AddonEffFs::SetColor(ColorF(0.9, 0.95, 1.0, 0.8));
			AddonEffFs::SetSpawnDensity(0.05, 0.25);
		}
		if (Key2.down())
		{
			currentKind = AddonEffFs::EffectKind::Rain;
			AddonEffFs::SetKind(currentKind);
			AddonEffFs::SetColor(ColorF(0.7, 0.8, 1.0, 0.9));
			AddonEffFs::SetSpawnDensity(0.15, 0.5);
		}

		if (KeyLeft.pressed())
		{
			windAngle -= Scene::DeltaTime() * 1.5;
		}
		if (KeyRight.pressed())
		{
			windAngle += Scene::DeltaTime() * 1.5;
		}

		const Vec2 dir = Vec2{ Math::Cos(windAngle), Math::Sin(windAngle) };
		AddonEffFs::SetDirection(dir);

		if (KeySpace.down())
		{
			AddonEffFs::AddImpulse(1.0, 0.7);
		}

		uiFont(U"[1] Wind  [2] Rain  [←→] Direction  [Space] Impulse").draw(20, 20, Palette::White);
		uiFont(U"Direction: {:.0f} deg"_fmt(Math::ToDegrees(windAngle))).draw(20, 46, Palette::White);
#pragma endregion
	}
}
