#include "BattleRendererHudInternal.h"

namespace BattleRendererHudInternal
{
	String GetCommandIconGlyph(const UnitArchetype archetype)
	{
		switch (archetype)
		{
		case UnitArchetype::Worker:
			return U"W";
		case UnitArchetype::Soldier:
			return U"S";
		case UnitArchetype::Archer:
			return U"A";
		case UnitArchetype::MachineGun:
			return U"M";
		case UnitArchetype::Healer:
			return U"+";
		case UnitArchetype::Barracks:
			return U"B";
		case UnitArchetype::Stable:
			return U"L";
		case UnitArchetype::Turret:
			return U"T";
		case UnitArchetype::Base:
			return U"H";
		case UnitArchetype::Spinner:
			return U"O";
		default:
			return U"?";
		}
	}

	ColorF GetCommandIconColor(const UnitArchetype archetype)
	{
		switch (archetype)
		{
		case UnitArchetype::Worker:
			return ColorF{ 0.22, 0.68, 0.42 };
		case UnitArchetype::Soldier:
			return ColorF{ 0.24, 0.48, 0.88 };
		case UnitArchetype::Archer:
			return ColorF{ 0.86, 0.56, 0.22 };
		case UnitArchetype::MachineGun:
			return ColorF{ 0.42, 0.74, 0.56 };
		case UnitArchetype::Healer:
			return ColorF{ 0.46, 0.82, 0.92 };
		case UnitArchetype::Barracks:
			return ColorF{ 0.56, 0.42, 0.74 };
		case UnitArchetype::Stable:
			return ColorF{ 0.64, 0.54, 0.28 };
		case UnitArchetype::Turret:
			return ColorF{ 0.78, 0.42, 0.30 };
		case UnitArchetype::Base:
			return ColorF{ 0.35, 0.60, 0.66 };
		case UnitArchetype::Spinner:
			return ColorF{ 0.92, 0.80, 0.26 };
		default:
			return ColorF{ 0.45, 0.48, 0.55 };
		}
	}

	String GetCommandKindLabel(const CommandKind kind)
	{
		switch (kind)
		{
		case CommandKind::Construction:
			return U"Build";
		case CommandKind::Upgrade:
			return U"Upgrade";
		case CommandKind::Production:
		default:
			return U"Queue";
		}
	}

	ColorF GetCommandAvailabilityColor(const CommandIconEntry& command)
	{
		if ((command.kind != CommandKind::Construction) && (command.kind != CommandKind::Upgrade))
		{
			return GetCommandIconColor(command.archetype);
		}

		return command.isEnabled
			? ColorF{ 0.30, 0.88, 0.48 }
			: ColorF{ 0.96, 0.34, 0.30 };
	}

	namespace
	{
		void DrawCommandTooltip(const CommandPanelLayout& layout, const CommandIconLayout& icon, const GameData& gameData)
		{
			const ColorF accentColor = ((icon.command.kind == CommandKind::Construction) || (icon.command.kind == CommandKind::Upgrade))
				? GetCommandAvailabilityColor(icon.command)
				: GetCommandIconColor(icon.command.archetype);
			const String titleText = icon.command.displayLabel.isEmpty() ? GetArchetypeLabel(icon.command.archetype) : icon.command.displayLabel;
			const bool hasDescription = !icon.command.descriptionText.isEmpty();
			const bool hasFlavor = !icon.command.flavorText.isEmpty();
			const double tooltipWidth = 320.0;
			double tooltipHeight = 74.0;
			if (hasDescription)
			{
				tooltipHeight += 18.0;
			}
			if (hasFlavor)
			{
				tooltipHeight += 18.0;
			}
			tooltipHeight += 18.0;
			const RectF tooltipRect{
				Clamp(icon.rect.center().x - (tooltipWidth * 0.5), 16.0, Scene::Width() - tooltipWidth - 16.0),
				Max(16.0, layout.panelRect.y - tooltipHeight - 10.0),
				tooltipWidth,
				tooltipHeight
			};

			RoundRect{ tooltipRect, 9 }.draw(ColorF{ 0.03, 0.05, 0.08, 0.96 });
			RoundRect{ tooltipRect, 9 }.drawFrame(2, 0, accentColor);
			gameData.smallFont(titleText).draw(tooltipRect.x + 12, tooltipRect.y + 10, Palette::White);
			gameData.smallFont(s3d::Format(GetCommandKindLabel(icon.command.kind), U" / ", icon.command.slot, U"   @", GetArchetypeLabel(icon.command.sourceArchetype))).draw(tooltipRect.x + 12, tooltipRect.y + 30, ColorF{ 0.84, 0.88, 0.94 });
			double lineY = tooltipRect.y + 50;
			gameData.smallFont(s3d::Format(U"Cost: ", icon.command.cost, U"G")).draw(tooltipRect.x + 12, lineY, Palette::Gold);
			lineY += 18.0;
			if (hasDescription)
			{
				gameData.smallFont(icon.command.descriptionText).draw(tooltipRect.x + 12, lineY, ColorF{ 0.96, 0.97, 0.99 });
				lineY += 18.0;
			}
			if (hasFlavor)
			{
				gameData.smallFont(icon.command.flavorText).draw(tooltipRect.x + 12, lineY, ColorF{ 0.72, 0.78, 0.86 });
				lineY += 18.0;
			}
			gameData.smallFont(icon.command.statusText).draw(
				tooltipRect.x + 12,
				lineY,
				icon.command.isEnabled ? ColorF{ 0.50, 0.92, 0.62 } : ColorF{ 1.0, 0.52, 0.46 });
		}

		void DrawCommandIcon(const RectF& rect, const CommandIconEntry& command, const GameData& gameData, const bool isHovered, const bool isPressed)
		{
			const String glyphText = command.glyphText.isEmpty() ? GetCommandIconGlyph(command.archetype) : command.glyphText;
			const String labelText = command.displayLabel.isEmpty() ? GetArchetypeLabel(command.archetype) : command.displayLabel;
			const double alpha = command.isEnabled ? 1.0 : 0.30;
			const ColorF iconColor{ GetCommandIconColor(command.archetype).r, GetCommandIconColor(command.archetype).g, GetCommandIconColor(command.archetype).b, alpha };
			const ColorF availabilityColor = GetCommandAvailabilityColor(command);
			const ColorF availabilityAlphaColor{ availabilityColor.r, availabilityColor.g, availabilityColor.b, command.isEnabled ? 0.95 : 0.85 };
			const RectF animatedRect = isPressed
				? RectF{ rect.x + 2, rect.y + 3, rect.w - 4, rect.h - 4 }
				: (isHovered ? RectF{ rect.x - 1, rect.y - 1, rect.w + 2, rect.h + 2 } : rect);
			const ColorF backgroundColor = isHovered
				? ColorF{ 0.16, 0.18, 0.24, 0.98 }
				: ColorF{ 0.10, 0.11, 0.15, 0.96 };
			const ColorF fillColor = command.isEnabled ? backgroundColor : ColorF{ 0.08, 0.09, 0.12, 0.94 };
			const ColorF textColor{ 1.0, 1.0, 1.0, alpha };
			const ColorF goldColor = (command.kind == CommandKind::Construction)
				? ColorF{ availabilityColor.r, availabilityColor.g, availabilityColor.b, alpha }
				: ColorF{ 1.0, 0.84, 0.0, alpha };
			RoundRect{ animatedRect, 10 }.draw(fillColor);
			RoundRect{ animatedRect, 10 }.drawFrame(isHovered ? 4 : 2, 0, iconColor);
			if ((command.kind == CommandKind::Construction) || (command.kind == CommandKind::Upgrade))
			{
				RoundRect{ animatedRect, 10 }.drawFrame(2, 0, availabilityAlphaColor);
				RectF{ animatedRect.x + 8, animatedRect.bottomY() - 10, animatedRect.w - 16, 5 }.draw(availabilityAlphaColor);
			}

			Circle{ animatedRect.x + 18, animatedRect.y + 18, 13 }.draw(iconColor);
			gameData.smallFont(Format(command.slot)).drawAt(animatedRect.x + 18, animatedRect.y + 18, textColor);

			Circle{ animatedRect.center().x, animatedRect.y + 34, 18 }.draw(iconColor);
			gameData.uiFont(glyphText).drawAt(animatedRect.center().x, animatedRect.y + 33, textColor);
			gameData.smallFont(labelText).drawAt(animatedRect.center().x, animatedRect.y + 61, textColor);
			gameData.smallFont(s3d::Format(command.cost, U"G")).drawAt(animatedRect.center().x, animatedRect.y + 79, goldColor);
		}
	}

	void DrawCommandSection(const CommandPanelLayout& layout, const GameData& gameData)
	{
		if (layout.commandIcons.isEmpty())
		{
			return;
		}

		const Vec2 cursorScreenPos = Cursor::PosF();
		const CommandIconLayout* hoveredIcon = nullptr;

		gameData.smallFont(layout.sectionLabel).draw(layout.panelRect.x + 16, layout.panelRect.y + 26, ColorF{ 0.82, 0.86, 0.92 });

		for (const auto& icon : layout.commandIcons)
		{
			const bool isHovered = icon.rect.intersects(cursorScreenPos);
			const bool isPressed = isHovered && MouseL.pressed() && icon.command.isEnabled;
			if (isHovered)
			{
				hoveredIcon = &icon;
			}

			DrawCommandIcon(icon.rect, icon.command, gameData, isHovered, isPressed);
		}

		if (hoveredIcon)
		{
			DrawCommandTooltip(layout, *hoveredIcon, gameData);
		}
	}
}
