//-----------------------------------------------
//
//	EffectsDetail.hpp
//	各効果実装用の共通 include
//
//-----------------------------------------------

# pragma once
# include "Effects.hpp"
# include "../UI/RectUI.hpp"

namespace pe
{
    inline void ApplyFullscreenEffect(const PixelShader& ps, const Texture& src)
    {
        const ScopedCustomShader2D shader{ ps };
        src.draw();
    }

    inline void ApplyFullscreenEffect(const PixelShader& ps, ConstantBuffer<Float4>& cb, const Float4& params, const Texture& src)
    {
        cb = params;
        Graphics2D::SetPSConstantBuffer(1, cb);
        ApplyFullscreenEffect(ps, src);
    }

    inline Effect MakePassthroughEffect()
    {
        Effect e;
        e.apply = [](const Texture& src, const EffectContext&)
        {
            src.draw();
        };
        e.drawUI = nullptr;
        e.reset = nullptr;
        return e;
    }

    inline Effect MakeSimpleShaderEffect(StringView shaderName, bool useEffectParams = false)
    {
        struct State
        {
            explicit State(StringView shader, const bool useParams)
                : ps{ LoadPS(shader, useParams) } {}

            PixelShader ps;
        };

        auto s = std::make_shared<State>(shaderName, useEffectParams);

        Effect e;
        e.apply = [s](const Texture& src, const EffectContext&)
        {
            ApplyFullscreenEffect(s->ps, src);
        };
        e.drawUI = nullptr;
        e.reset = nullptr;
        return e;
    }
}
