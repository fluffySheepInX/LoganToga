# pragma once
# include <Siv3D.hpp>
# include <algorithm>
# include <memory>
# include "../Editors/IEditorAddon.hpp"

namespace app
{
    class EditorAddonHost
    {
    public:
        void registerAddon(std::unique_ptr<IEditorAddon> addon)
        {
            if (addon)
            {
                m_addons << std::move(addon);
            }
        }

        void update(const EditorUpdateContext& context)
        {
            for (auto* addon : addonsByUpdateOrder())
            {
                addon->update(context);
            }

            m_hoveredEditorId = findHoveredEditorId();
            if (m_hoveredEditorId)
            {
                m_focusedEditorId = m_hoveredEditorId;
                if (not m_activeEditorId)
                {
                    m_activeEditorId = m_hoveredEditorId;
                }
            }

            if (KeyS.down())
            {
                dispatchCommandToActive(EditorCommand::Save);
            }
            if (KeyL.down())
            {
                dispatchCommandToActive(EditorCommand::Load);
            }
            if (KeyControl.pressed() && KeyZ.down())
            {
                dispatchCommandToActive(EditorCommand::Undo);
            }
            if (KeyControl.pressed() && KeyD.down())
            {
                dispatchCommandToActive(EditorCommand::Duplicate);
            }
            if (KeyDelete.down())
            {
                dispatchCommandToActive(EditorCommand::DeleteSelection);
            }
            if (KeyEnter.down())
            {
                dispatchCommandToActive(EditorCommand::Confirm);
            }
            if (KeyEscape.down())
            {
                dispatchCommandToActive(EditorCommand::Cancel);
            }
            if (KeyG.down())
            {
                dispatchCommandToActive(EditorCommand::ToggleGhost);
            }

            for (auto* addon : addonsByInputPriority())
            {
                if (const auto& toggleShortcut = addon->descriptor().toggleShortcut;
                    toggleShortcut && toggleShortcut->down())
                {
                    setActiveEditor(addon->id());
                    dispatchCommandToActive(EditorCommand::Toggle);
                    break;
                }
            }
        }

        void draw3D(const EditorDraw3DContext& context)
        {
            for (auto* addon : addonsByDraw3DOrder())
            {
                addon->draw3D(context);
            }
        }

        void drawUI(const EditorUIContext& context)
        {
            for (auto* addon : addonsByDrawUIOrder())
            {
                addon->drawUI(context);
            }
        }

        bool wantsMouseCapture() const
        {
            for (const auto& addon : m_addons)
            {
                if (addon->isEnabled() && addon->wantsMouseCapture())
                {
                    return true;
                }
            }

            return false;
        }

        bool wantsMouseWheelCapture() const
        {
            for (const auto& addon : m_addons)
            {
                if (addon->isEnabled() && addon->wantsMouseWheelCapture())
                {
                    return true;
                }
            }

            return false;
        }

        Optional<String> activeEditorId() const
        {
            return m_activeEditorId;
        }

        Optional<String> focusedEditorId() const
        {
            return m_focusedEditorId;
        }

        Optional<String> hoveredEditorId() const
        {
            return m_hoveredEditorId;
        }

        void setActiveEditor(StringView id)
        {
            for (const auto& addon : m_addons)
            {
                if (addon->id() == id)
                {
                    m_activeEditorId = String{ id };
                    m_focusedEditorId = m_activeEditorId;
                    return;
                }
            }
        }

        bool dispatchCommandToActive(const EditorCommand command)
        {
            if (const auto targetEditorId = commandTargetEditorId())
            {
                for (auto& addon : m_addons)
                {
                    if (addon->id() == *targetEditorId)
                    {
                        if (addon->handleCommand(command))
                        {
                            return true;
                        }

                        return false;
                    }
                }
            }

            return false;
        }

        bool broadcastCommand(const EditorCommand command)
        {
            for (auto& addon : m_addons)
            {
                if (addon->handleCommand(command))
                {
                    return true;
                }
            }

            return false;
        }

    private:
        template <class OrderSelector>
        Array<IEditorAddon*> addonsSortedBy(OrderSelector orderSelector) const
        {
            Array<IEditorAddon*> sorted;
            sorted.reserve(m_addons.size());

            for (const auto& addon : m_addons)
            {
                sorted << addon.get();
            }

            std::stable_sort(sorted.begin(), sorted.end(), [&](const IEditorAddon* a, const IEditorAddon* b)
            {
                return orderSelector(a->descriptor()) < orderSelector(b->descriptor());
            });

            return sorted;
        }

        Array<IEditorAddon*> addonsByUpdateOrder() const
        {
            return addonsSortedBy([](const EditorAddonDescriptor& descriptor)
            {
                return descriptor.updateOrder;
            });
        }

        Array<IEditorAddon*> addonsByDraw3DOrder() const
        {
            return addonsSortedBy([](const EditorAddonDescriptor& descriptor)
            {
                return descriptor.draw3DOrder;
            });
        }

        Array<IEditorAddon*> addonsByDrawUIOrder() const
        {
            return addonsSortedBy([](const EditorAddonDescriptor& descriptor)
            {
                return descriptor.drawUIOrder;
            });
        }

        Array<IEditorAddon*> addonsByInputPriority() const
        {
            return addonsSortedBy([](const EditorAddonDescriptor& descriptor)
            {
                return -descriptor.inputPriority;
            });
        }

        Optional<String> findHoveredEditorId() const
        {
            for (auto* addon : addonsByInputPriority())
            {
                if (addon->isEnabled() && addon->isPanelOpen() && addon->wantsMouseCapture())
                {
                    return String{ addon->id() };
                }
            }

            return none;
        }

        Optional<String> commandTargetEditorId() const
        {
            if (m_focusedEditorId)
            {
                return m_focusedEditorId;
            }

            return m_activeEditorId;
        }

        Array<std::unique_ptr<IEditorAddon>> m_addons;
        Optional<String> m_activeEditorId;
        Optional<String> m_focusedEditorId;
        Optional<String> m_hoveredEditorId;
    };
}
