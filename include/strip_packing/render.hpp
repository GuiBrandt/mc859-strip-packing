#ifndef STRIP_PACKING_RENDER_HPP
#define STRIP_PACKING_RENDER_HPP

#include "defs.hpp"

#include <string>

#include <blend2d.h>

namespace strip_packing::render {

class solution_renderer {
  private:
    constexpr static BLRgba32 WHITE = BLRgba32(0xFFFFFFFF);
    constexpr static BLRgba32 BLACK = BLRgba32(0xFF000000);

    const instance_t& m_instance;
    const solution_t& m_solution;

    dim_type m_recipient_height;
    dim_type m_max_weight;

    double figure_width(double scale) const {
        return m_instance.recipient_length * scale;
    }

    double figure_height(double scale) const {
        return m_recipient_height * scale;
    }

    /*! Desenha o recipiente. */
    void render_recipient(BLContext& ctx, double scale, double x,
                          double y) const {
        ctx.setStrokeWidth(4);
        BLRect rect(x, y, figure_width(scale), figure_height(scale));
        ctx.fillRect(rect, WHITE);
        ctx.strokeRect(rect, BLACK);
    }

    /*! Desenha um nível da solução. */
    double render_level(BLContext& ctx, double scale, double x, double y,
                        const instance_t::rect_subset& level) const {
        ctx.setStrokeWidth(1);
        dim_type level_height = 0;
        for (auto i : level) {
            auto rect = m_instance.rects[i];
            x = render_rect(ctx, scale, x, y, rect);
            level_height = std::max(level_height, rect.height);
        }
        return y - level_height * scale;
    }

    /*! Desenha um retângulo da solução. */
    double render_rect(BLContext& ctx, double scale, double x, double y,
                       const rect_t& rect) const {
        auto [length, height, weight] = rect;

        BLRect bl_rect(x, y - height * scale, length * scale, height * scale);

        // Preenche o retângulo com vermelho, em intensidade determinada pelo
        // peso do retângulo.
        double alpha =
            std::round(0xFF * (m_max_weight > 0 ? weight / m_max_weight : 1));
        BLRgba32 redscale(0xFF, 0x00, 0x00, alpha);
        ctx.fillRect(bl_rect, redscale);
        ctx.strokeRect(bl_rect, BLACK);

        return x + length * scale;
    }

  public:
    solution_renderer(const instance_t& instance, const solution_t& solution)
        : m_instance(instance), m_solution(solution), m_max_weight(0) {
        // Computa a altura total da solução, que será usada durante a
        // renderização.
        dim_type height = 0;
        for (const auto& level : m_solution) {
            dim_type level_height = 0;
            for (auto i : level) {
                auto rect = m_instance.rects[i];
                level_height = std::max(level_height, rect.height);
                m_max_weight = std::max(m_max_weight, rect.weight);
            }
            height += level_height;
        }
        m_recipient_height = height;
    }

    /*! Desenha uma solução e salva em um arquivo. */
    void render(std::string filename, double scale = 8,
                double horz_padding = 24, double vert_padding = 24) const {

        if (figure_height(scale) > 5000) {
            scale = 5000.0 / m_recipient_height;
        }

        double img_width = std::ceil(figure_width(scale) + horz_padding * 2);
        double img_height = std::ceil(figure_height(scale) + vert_padding * 2);

        BLImage img(img_width, img_height, BL_FORMAT_PRGB32);

        BLContext ctx(img);
        ctx.clearAll();

        render_recipient(ctx, scale, horz_padding, vert_padding);

        double y = img_height - vert_padding;
        for (const auto& level : m_solution) {
            y = render_level(ctx, scale, horz_padding, y, level);

            // Desenha uma linha separadora entre os níveis.
            BLLine line(horz_padding, y, horz_padding + figure_width(scale), y);
            ctx.setStrokeWidth(2);
            ctx.strokeLine(line, BLACK);
        }

        ctx.end();
        img.writeToFile(filename.c_str());
    }
};

static void render_solution(const instance_t& instance, solution_t solution,
                            std::string filename) {
    solution_renderer(instance, solution).render(filename);
}

}; // namespace strip_packing::render

#endif // STRIP_PACKING_RENDER_HPP
