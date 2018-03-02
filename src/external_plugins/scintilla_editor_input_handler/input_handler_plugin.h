#pragma once

class DefaultInputHandler
        : public virtual PluginBase
        , public virtual InputHandler
{
public:
    DefaultInputHandler() :
        PluginBase("scintilla_editor_input_handler", "scintilla editor input handler plugin for keyboard and mouse", 1)
    {
    }

    virtual ~DefaultInputHandler() = default;

    MultipleInstancePluginPtr NewInstance() override {
        return MultipleInstancePluginPtr{new DefaultInputHandler};
    }

private:
protected:
public:
    bool ProcessCharInput(int32_t codepoint, InputHandler::ModifierEnum modifier) override;
    bool ProcessKey(InputHandler::KeyCodeEnum key, InputHandler::ModifierEnum mods, bool down) override;
    bool ProcessMouseButton(InputHandler::MouseButtonEnum btn, uint32_t col, uint32_t row, InputHandler::ModifierEnum modifier, bool down) override;
    bool ProcessMouseMove(InputHandler::MouseButtonEnum btn, uint32_t col, uint32_t row, InputHandler::ModifierEnum modifier) override;
private:
    void SendMouseEvent(int button, bool press, bool motion, uint32_t col, uint32_t row);
};
