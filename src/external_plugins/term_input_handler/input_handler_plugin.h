#pragma once

class DefaultInputHandler
        : public virtual PluginBase
        , public virtual InputHandler
{
public:
    DefaultInputHandler() :
        PluginBase("default_term_input_handler", "default input handler plugin for keyboard and mouse", 1)
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

    bool ProcessMouseButton(InputHandler::MouseButtonEnum btn, uint32_t x, uint32_t y, InputHandler::ModifierEnum modifier, bool down) override {
        (void)btn;
        (void)x;
        (void)y;
        (void)modifier;
        (void)down;
        return false;
    }

    bool ProcessMouseMove(InputHandler::MouseButtonEnum btn, uint32_t x, uint32_t y, InputHandler::ModifierEnum modifier) override {
        (void)btn;
        (void)x;
        (void)y;
        (void)modifier;
        return false;
    }
private:
};
