#pragma once

#include <vulkan/vulkan.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <array>

namespace ppr
{
    struct vertex
    {
    public:
        vertex(glm::vec2 a_position,
               glm::vec3 a_color)
            : position(a_position)
            , color(a_color)
        {}
        const glm::vec2 position;
        const glm::vec3 color;

    public:
        static const vk::VertexInputBindingDescription binding_descript();
        static const std::array<vk::VertexInputAttributeDescription, 2> attribute_descript();
    };
}