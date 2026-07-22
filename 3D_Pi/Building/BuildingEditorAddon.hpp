# pragma once
# include <Siv3D.hpp>
# include "../Editors/IEditorAddon.hpp"
# include "BuildingDocument.hpp"
# include "BuildingEditorPanel.hpp"
# include "BuildingPlacementTool.hpp"
# include "BuildingRenderer.hpp"
# include "BuildingSerializer.hpp"

namespace app
{
	class BuildingEditorAddon final : public IEditorAddon
	{
	public:
		const EditorAddonDescriptor& descriptor() const noexcept override
		{
			static const EditorAddonDescriptor descriptor{
				U"BuildingEditor",
				U"Building Builder",
				Optional<Input>{ KeyB },
				0,
				0,
				0,
				0
			};
			return descriptor;
		}

		void update(const EditorUpdateContext& context) override
		{
			if (context.uiHidden)
			{
				m_panel.syncCollapsedIconRegistry();
				return;
			}

			m_tool.update(m_document, context.camera, context.cursorBlockedByUI);
		}

		void draw3D(const EditorDraw3DContext& context) override
		{
			m_renderer.draw(m_document, m_tool.selectedBuildingId(), context.uiHidden ? none : m_tool.placementGhost());
		}

		void drawUI(const EditorUIContext& context) override
		{
			if (context.uiHidden)
			{
				m_panel.syncCollapsedIconRegistry();
				return;
			}

			auto actions = m_panel.draw(m_document, m_tool.selectedBuildingId(), m_status);
			if (actions.selectedBuildingId != m_tool.selectedBuildingId())
			{
				m_tool.setSelectedBuildingId(actions.selectedBuildingId);
			}
			applyActions(actions);
		}

		bool wantsMouseCapture() const override
		{
			return m_panel.wantsMouseCapture();
		}

		bool wantsMouseWheelCapture() const override
		{
			return m_panel.wantsMouseWheelCapture();
		}

		bool isEnabled() const override
		{
			return true;
		}

		bool isPanelOpen() const override
		{
			return m_panel.isOpen();
		}

		bool handleCommand(const EditorCommand command) override
		{
			switch (command)
			{
			case EditorCommand::Toggle:
				m_panel.toggleOpen();
				return true;
			case EditorCommand::Save:
				save();
				return true;
			case EditorCommand::Load:
				load();
				return true;
			case EditorCommand::Duplicate:
				m_tool.duplicateSelected(m_document);
				return true;
			case EditorCommand::DeleteSelection:
				m_tool.deleteSelected(m_document);
				return true;
			case EditorCommand::Cancel:
			case EditorCommand::CancelTransientTool:
				m_tool.cancelTransientTool();
				return true;
			default:
				return false;
			}
		}

	private:
		static constexpr StringView DefaultSavePath = U"data/buildings.toml";

		void applyActions(const building::BuildingPanelActions& actions)
		{
			if (actions.beginPlacement)
			{
				m_tool.beginPlacement();
				m_status = U"Click ground to place building";
			}
			if (actions.duplicateSelected)
			{
				m_tool.duplicateSelected(m_document);
				m_status = U"Duplicated building";
			}
			if (actions.deleteSelected)
			{
				m_tool.deleteSelected(m_document);
				m_status = U"Deleted building";
			}
			if (actions.rotateLeft)
			{
				m_tool.rotateSelected(m_document, -0.03125);
			}
			if (actions.rotateRight)
			{
				m_tool.rotateSelected(m_document, 0.03125);
			}
			if (actions.rerollSeed)
			{
				if (auto id = m_tool.selectedBuildingId())
				{
					if (auto* selected = m_document.findById(*id))
					{
						selected->variationSeed = static_cast<uint32>(Time::GetMillisec() & 0xffffffffu);
						m_status = U"Rerolled variation seed";
					}
				}
			}
			if (actions.save)
			{
				save();
			}
			if (actions.load)
			{
				load();
			}
			if (actions.cancel)
			{
				m_tool.cancelTransientTool();
			}
		}

		void save()
		{
			building::SaveBuildingDocument(FilePath{ DefaultSavePath }, m_document);
			m_status = U"Saved data/buildings.toml";
		}

		void load()
		{
			if (building::LoadBuildingDocument(FilePath{ DefaultSavePath }, m_document))
			{
				m_tool.setSelectedBuildingId(none);
				m_status = U"Loaded data/buildings.toml";
			}
			else
			{
				m_status = U"Failed to load data/buildings.toml";
			}
		}

		building::BuildingDocument m_document;
		building::BuildingPlacementTool m_tool;
		building::BuildingRenderer m_renderer;
		building::BuildingEditorPanel m_panel;
		String m_status = U"Ready";
	};
}
