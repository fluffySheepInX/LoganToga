#include "../AddonGaussian.h"

/// @brief 
/// @param size 
void GaussianFSAddon::Condition(const Point& size, const ColorF argC, const ColorF argBarC)
{
	if (auto p = Addon::GetAddon<GaussianFSAddon>(U"GaussianFSAddon"))
	{
		// 背景色の設定
		Scene::SetBackground(argC);

		System::SetTerminationTriggers(UserAction::CloseButtonClicked);

		// ウィンドウバーの色設定
		p->m_barColor = argBarC;
		// ウィンドウの枠を非表示にする
		Window::SetStyle(WindowStyle::Frameless);
		// 中央に配置
		Window::Centering();
		// ウィンドウサイズ変更
		Window::Resize(size);

#pragma region 画面サイズ設定など
		// ガウスぼかし用テクスチャのサイズ変更
		// 画面サイズと同じ
		p->sceneSize = size;
		p->gaussianA1 = RenderTexture{ p->sceneSize };
		p->gaussianB1 = RenderTexture{ p->sceneSize };
		p->gaussianA4 = RenderTexture{ p->sceneSize / 4 };
		p->gaussianB4 = RenderTexture{ p->sceneSize / 4 };
		p->gaussianA8 = RenderTexture{ p->sceneSize / 8 };
		p->gaussianB8 = RenderTexture{ p->sceneSize / 8 };
#pragma endregion
	}
}
double GaussianFSAddon::GetSCALE()
{
	if (auto p = Addon::GetAddon<GaussianFSAddon>(U"GaussianFSAddon"))
	{
		return p->m_SCALE;
	}
	else
	{
		return 1.0;
	}
}
Vec2 GaussianFSAddon::GetOFFSET()
{
	if (auto p = Addon::GetAddon<GaussianFSAddon>(U"GaussianFSAddon"))
	{
		return p->m_OFFSET;
	}
	else
	{
		return Vec2{ 0.0, 0.0 };
	}
}
/// @brief アクティブかどうか
/// @return 
[[nodiscard]]
bool GaussianFSAddon::IsActive()
{
	if (auto p = Addon::GetAddon<GaussianFSAddon>(U"GaussianFSAddon"))
	{
		return p->m_active;
	}
	else
	{
		return false;
	}
}
/// @brief アクティブにします
void GaussianFSAddon::SetActive()
{
	if (auto p = Addon::GetAddon<GaussianFSAddon>(U"GaussianFSAddon"))
	{
		p->m_active = true;
	}
}
/// @brief 非アクティブにします
void GaussianFSAddon::SetNotActive()
{
	if (auto p = Addon::GetAddon<GaussianFSAddon>(U"GaussianFSAddon"))
	{
		p->m_active = false;
	}
}
void GaussianFSAddon::SetLangSet(const Array<std::pair<String, String>>& langSet)
{
	if (auto p = Addon::GetAddon<GaussianFSAddon>(U"GaussianFSAddon"))
	{
		p->m_htLang.clear();
		for (const auto& [key, value] : langSet) { p->m_htLang.emplace(key, value); }
	}
}
void GaussianFSAddon::SetLang(const String lang)
{
	if (auto p = Addon::GetAddon<GaussianFSAddon>(U"GaussianFSAddon"))
	{
		p->m_lastLang = lang;
	}
}
String GaussianFSAddon::GetLang()
{
	if (auto p = Addon::GetAddon<GaussianFSAddon>(U"GaussianFSAddon"))
	{
		return p->m_lastLang;
	}
	return String{};
}
void GaussianFSAddon::SetSceneSet(const Array<Array<String>>& langSet)
{
	if (auto p = Addon::GetAddon<GaussianFSAddon>(U"GaussianFSAddon"))
	{
		p->m_arraySceneSize = langSet;
	}
}
void GaussianFSAddon::SetScene(const String lang)
{
	if (auto p = Addon::GetAddon<GaussianFSAddon>(U"GaussianFSAddon"))
	{
		p->m_lastSceneSize = lang;
	}
}

/// @brief 
/// @param dragStart 
/// @param ww 
void GaussianFSAddon::DragProcessWindow()
{
	if (auto p = Addon::GetAddon<GaussianFSAddon>(U"GaussianFSAddon"))
	{
		if (p->m_lockWindow) return;

#pragma region ドラッグ処理
		if (Rect{ 0, 0, p->sceneSize.x, 60 }.mouseOver() == true)
			Cursor::RequestStyle(U"MyCursorHand");
		if (p->m_dragStartWindow)
			(MouseL.pressed() == true)
			? Window::SetPos(p->m_dragStartWindow->second + (Cursor::ScreenPos() - p->m_dragStartWindow->first)) : p->m_dragStartWindow.reset();
		// ドラッグの開始
		if (Rect{ 0, 0, p->sceneSize.x, 60 }.leftClicked())
			p->m_dragStartWindow = { Cursor::ScreenPos(), Window::GetState().bounds.pos };
#pragma endregion
	}
}

bool GaussianFSAddon::TriggerOrDisplayESC()
{
	if (auto p = Addon::GetAddon<GaussianFSAddon>(U"GaussianFSAddon"))
	{
		const Transformer2D screenScaling{ Mat3x2::Scale(p->m_SCALE).translated(p->m_OFFSET), TransformCursor::Yes };

		if (p->m_escPressed)
		{
			// フルスクリーン半透明オーバーレイ
			RectF(Scene::Width(), Scene::Height()).draw(ColorF(0.0, 0.55));

			// ダイアログ本体（中央）
			constexpr double W = 420;
			constexpr double H = 160;
			const Vec2 center = Vec2(Scene::Width(), Scene::Height()) / 2.0;
			const RectF dialogRect(center.x - W / 2.0, center.y - H / 2.0, W, H);
			dialogRect.rounded(8).draw(ColorF(0.12));
			dialogRect.drawFrame(2, Palette::White);

			// メッセージ
			const String msg = U"本当に終了しますか？";
			p->m_font(msg).drawAt(dialogRect.center().x, dialogRect.y + 44, Palette::White);

			// ボタン
			const RectF yesRect(dialogRect.x + 40, dialogRect.y + dialogRect.h - 56, 140, 40);
			const RectF noRect(dialogRect.x + dialogRect.w - 40 - 140, dialogRect.y + dialogRect.h - 56, 140, 40);

			// ボタンの見た目
			if (yesRect.mouseOver()) yesRect.rounded(6).draw(ColorF(0.25)); else yesRect.rounded(6).draw(ColorF(0.18));
			if (noRect.mouseOver()) noRect.rounded(6).draw(ColorF(0.25)); else noRect.rounded(6).draw(ColorF(0.18));
			p->m_font(U"終了する").drawAt(yesRect.center(), Palette::White);
			p->m_font(U"キャンセル").drawAt(noRect.center(), Palette::White);

			// クリック / キー操作
			if (yesRect.leftClicked() || KeyEnter.down())
			{
				// 確定でループ脱出（アプリ終了）
				return true;
			}
			if (noRect.leftClicked() || KeyEscape.down())
			{
				// キャンセル
				p->m_escPressed = false;
			}

			return false;
		}

		if (KeyEscape.down())
		{
			p->m_escPressed = true;
		}
		return false;
	}
}
bool GaussianFSAddon::TriggerOrDisplayLang()
{
	if (auto p = Addon::GetAddon<GaussianFSAddon>(U"GaussianFSAddon"))
	{
		const Transformer2D screenScaling{ Mat3x2::Scale(p->m_SCALE).translated(p->m_OFFSET), TransformCursor::Yes };

		if (p->m_langPressed)
		{
			int32 yOffset = 10;
			int32 counter = 1;
			Array<std::pair<Rect, String>> langArray;
			for (const auto& elem : p->m_htLang)
			{
				Rect tempRe{ 10,(p->sceneSize.y - 25) - (20 * counter) - yOffset,60,20 };
				tempRe.draw(Palette::Black);
				p->m_font(elem.second).drawAt(tempRe.center(), Palette::White);
				langArray.push_back({ tempRe, elem.first });
				counter++;
			}

			for (const auto& da : langArray)
			{
				if (da.first.leftClicked())
				{
					p->m_lastLang = da.second;
				}
			}

			if (MouseL.down())
			{
				p->m_langPressed = false;
			}
		}
		else
		{
			Rect langBtnRect{ 10, p->sceneSize.y - 25, 60, 20 };
			if (langBtnRect.leftClicked())
			{
				p->m_langPressed = true;
			}
		}
	}
	return false;
}
bool GaussianFSAddon::TriggerOrDisplaySceneSize()
{
	if (auto p = Addon::GetAddon<GaussianFSAddon>(U"GaussianFSAddon"))
	{
		const Transformer2D screenScaling{ Mat3x2::Scale(p->m_SCALE).translated(p->m_OFFSET), TransformCursor::Yes };

		if (p->m_sceneSizePressed)
		{
			int32 yOffset = 10;
			int32 counter = 1;
			Array<std::pair<Rect, String>> langArray;
			for (const auto& elem : p->m_arraySceneSize)
			{
				Rect tempRe{ 10 + 60 + 10,(p->sceneSize.y - 25) - (20 * counter) - yOffset,60,20 };
				tempRe.draw(Palette::Black);
				p->m_font(elem[0]).drawAt(tempRe.center(), Palette::White);
				langArray.push_back({ tempRe, elem[0] });
				counter++;
			}

			for (const auto& da : langArray)
			{
				if (da.first.leftClicked())
				{
					p->m_lastSceneSize = da.second;

					for (auto& da2 : p->m_arraySceneSize)
					{
						if (da2[0] == da.second)
						{
							
							const Size BaseSceneSize{ 1600, 900 };
							const Size BaseSceneSize001{ Parse<int32>(da2[1]), Parse<int32>(da2[2]) };
							p->m_SCALE = p->CalculateScale(BaseSceneSize, BaseSceneSize001);
							p->m_OFFSET = p->CalculateOffset(BaseSceneSize, BaseSceneSize001);
							p->SetWindSize(BaseSceneSize001.x, BaseSceneSize001.y);
						}
					}
				}
			}

			if (MouseL.down())
			{
				p->m_sceneSizePressed = false;
			}
		}
		else
		{
			Rect langBtnRect{ 10 + 60 + 10, p->sceneSize.y - 25, 60, 20 };
			if (langBtnRect.leftClicked())
			{
				p->m_sceneSizePressed = true;
			}
		}
	}
	return false;
}

/// @brief 
/// @return 
Point GaussianFSAddon::GetWindowSize()
{
	if (auto p = Addon::GetAddon<GaussianFSAddon>(U"GaussianFSAddon"))
	{
		return p->sceneSize;
	}
	return Point();
}
/// @brief 
/// @return 
Point GaussianFSAddon::GetInitWindowSize()
{
	if (auto p = Addon::GetAddon<GaussianFSAddon>(U"GaussianFSAddon"))
	{
		return p->initSceneSize;
	}
	return Point();
}
/// @brief 
/// @return 
bool GaussianFSAddon::IsGameEnd()
{
	if (auto p = Addon::GetAddon<GaussianFSAddon>(U"GaussianFSAddon"))
	{
		const Transformer2D screenScaling{ Mat3x2::Scale(p->m_SCALE).translated(p->m_OFFSET), TransformCursor::Yes };

		if (p->m_NOWSCENE == U"SelectLang" || p->m_NOWSCENE == U"WinSizeScene")
		{
			if (Shape2D::Stairs(Vec2{ p->initSceneSize.x - 5, p->initSceneSize.y - 5 }, 20, 20, 4).asPolygon().leftClicked())
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		else
		{
			if (Shape2D::Stairs(Vec2{ p->sceneSize.x - 5, p->sceneSize.y - 5 }, 20, 20, 4).asPolygon().leftClicked())
			{
				return true;
			}
			else
			{
				return false;
			}
		}
	}
	return true;
}
bool GaussianFSAddon::IsHide()
{
	if (auto p = Addon::GetAddon<GaussianFSAddon>(U"GaussianFSAddon"))
	{
		const Transformer2D screenScaling{ Mat3x2::Scale(p->m_SCALE).translated(p->m_OFFSET), TransformCursor::Yes };

		if (p->m_NOWSCENE == U"SelectLang" || p->m_NOWSCENE == U"WinSizeScene")
		{
			return false;
		}

		if (RectF{ Vec2{ p->sceneSize.x - 50, p->sceneSize.y - 20 }
			, 15, 10 }
			.leftClicked())
		{
			return true;
		}
	}
	return false;
}
/// @brief 
/// @param argS 
void GaussianFSAddon::SetSceneName(const String argS)
{
	if (auto p = Addon::GetAddon<GaussianFSAddon>(U"GaussianFSAddon"))
	{
		p->m_NOWSCENE = argS;
	}
}

/// @brief 
/// @return 
[[nodiscard]]
bool GaussianFSAddon::update()
{
	if (not m_active)
		return true;
	return true;
}
/// @brief 
void GaussianFSAddon::draw() const
{
	if (not m_active)
		return;

	Draw000();
	Draw001();
	ProcessMultiLang();
	ProcessSizeChange();

	// ガウスぼかし用テクスチャにもう一度シーンを描く
	{
		const ScopedRenderTarget2D target{ gaussianA1.clear(ColorF{ 0.0 }) };
		const ScopedRenderStates2D blend{ BlendState::Additive };
		Draw000();
		Draw001();
		ProcessMultiLang();
		ProcessSizeChange();
	}

	// オリジナルサイズのガウスぼかし (A1)
	// A1 を 1/4 サイズにしてガウスぼかし (A4)
	// A4 を 1/2 サイズにしてガウスぼかし (A8)
	Shader::GaussianBlur(gaussianA1, gaussianB1, gaussianA1);
	Shader::Downsample(gaussianA1, gaussianA4);
	Shader::GaussianBlur(gaussianA4, gaussianB4, gaussianA4);
	Shader::Downsample(gaussianA4, gaussianA8);
	Shader::GaussianBlur(gaussianA8, gaussianB8, gaussianA8);

	{
		const ScopedRenderStates2D blend{ BlendState::Additive };
		if (a1) gaussianA1.resized(sceneSize).draw(ColorF{ a1 });
		if (a4) gaussianA4.resized(sceneSize).draw(ColorF{ a4 });
		if (a8) gaussianA8.resized(sceneSize).draw(ColorF{ a8 });
	}
}
/// @brief 
void GaussianFSAddon::Draw000() const
{
	ColorF col = m_barColor;

	if (m_NOWSCENE == U"SelectLang" || m_NOWSCENE == U"WinSizeScene")
	{
		//right area
		Rect{ initSceneSize.x - 1, 0, 1, initSceneSize.y }.draw(col);
		//top area
		Rect{ 0, 0, initSceneSize.x, 1 }.draw(col);
		//left area
		Rect{ 0, 0, 1, initSceneSize.y }.draw(col);
		//bottom area
		Rect{ 0, initSceneSize.y - 30, initSceneSize.x, 30 }.draw(col);
	}
	else
	{
		const Transformer2D screenScaling{ Mat3x2::Scale(m_SCALE).translated(m_OFFSET), TransformCursor::Yes };
		//right area
		Rect{ sceneSize.x - 1, 0, 1, sceneSize.y }.draw(col);
		//top area
		Rect{ 0, 0, sceneSize.x, 1 }.draw(col);
		//left area
		Rect{ 0, 0, 1, sceneSize.y }.draw(col);
		//bottom area
		Rect{ 0, sceneSize.y - 30, sceneSize.x, 30 }.draw(col);
	}
}
/// @brief 
void GaussianFSAddon::Draw001() const
{
#pragma region exit button
	if (m_NOWSCENE == U"SelectLang" || m_NOWSCENE == U"WinSizeScene")
	{
		Shape2D::Stairs(Vec2{ initSceneSize.x - 5, initSceneSize.y - 5 }, 20, 20, 4).draw(Palette::Black);
	}
	else
	{
		const Transformer2D screenScaling{ Mat3x2::Scale(m_SCALE).translated(m_OFFSET), TransformCursor::Yes };
		Shape2D::Stairs(Vec2{ sceneSize.x - 5, sceneSize.y - 5 }, 20, 20, 4).draw(Palette::Black);
		Line{ Vec2{ sceneSize.x - 50, sceneSize.y - 15 }
			, Vec2{ sceneSize.x - 35, sceneSize.y - 15 } }
		.draw(5, Palette::Black);
	}
#pragma endregion
}

void GaussianFSAddon::ProcessMultiLang() const
{
	const Transformer2D screenScaling{ Mat3x2::Scale(m_SCALE).translated(m_OFFSET), TransformCursor::Yes };

	Rect langBtnRect{ 10, sceneSize.y - 25, 60, 20 };
	langBtnRect.draw(Palette::Black);
	if (auto it = m_htLang.find(m_lastLang); it != m_htLang.end())
	{
		m_font(it->second).drawAt(langBtnRect.center(), Palette::White);
	}
}
void GaussianFSAddon::ProcessSizeChange() const
{
	const Transformer2D screenScaling{ Mat3x2::Scale(m_SCALE).translated(m_OFFSET), TransformCursor::Yes };

	Rect langBtnRect{ 10, sceneSize.y - 25, 60, 20 };
	langBtnRect.movedBy(langBtnRect.w + 10, 0).draw(Palette::Black);
	m_font(m_lastSceneSize).drawAt(langBtnRect.movedBy(langBtnRect.w + 10, 0).center(), Palette::White);
}

void GaussianFSAddon::SetWindSize(int32 w, int32 h)
{
	// ゲームのシーンサイズ
	const Size sceneSize{ w, h };
	// 必要なシーンサイズよりやや大きめの領域（タイトルバーやフレームを考慮）
	const Size requiredAreadSize{ sceneSize + Size{ 60, 10 } };
	// プレイヤーのワークエリア（画面サイズからタスクバーを除いた領域）のサイズ
	const Size workAreaSize = System::GetCurrentMonitor().workArea.size;
	// OS の UI のスケール（多くの場合 1.0～2.0）
	const double uiScaling = Window::GetState().scaling;
	// UI スケールを考慮したワークエリアサイズ
	const Size availableWorkAreaSize = (SizeF{ workAreaSize } / uiScaling).asPoint();
	// ゲームのシーンサイズがプレイヤーのワークエリア内に収まるか
	const bool ok = (requiredAreadSize.x <= availableWorkAreaSize.x) && (requiredAreadSize.y <= availableWorkAreaSize.y);

	if (ok)
	{
		Window::Resize(sceneSize);
	}
	else
	{
		// UI 倍率 1.0 相当でリサイズ
		Scene::SetResizeMode(ResizeMode::Keep);
		Scene::Resize(sceneSize);
		Window::ResizeActual(sceneSize);
	}
}
// オリジナルのシーンを何倍すればよいかを返す関数
double GaussianFSAddon::CalculateScale(const Vec2& baseSize, const Vec2& currentSize)
{
	return Min((currentSize.x / baseSize.x), (currentSize.y / baseSize.y));
}
// 画面の中央に配置するためのオフセットを返す関数
Vec2 GaussianFSAddon::CalculateOffset(const Vec2& baseSize, const Vec2& currentSize)
{
	return ((currentSize - baseSize * CalculateScale(baseSize, currentSize)) / 2.0);
}
