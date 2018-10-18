#include "vertex.hpp"

namespace ppr
{
    const vk::VertexInputBindingDescription vertex::binding_descript()
    {
        return vk::VertexInputBindingDescription(0, sizeof(vertex), vk::VertexInputRate::eVertex);
    }

    const std::array<vk::VertexInputAttributeDescription, 2> vertex::attribute_descript()
    {
        std::array<vk::VertexInputAttributeDescription, 2> attributes;

        attributes[0].binding = 0;
        attributes[0].location = 0;
        attributes[0].format = vk::Format::eR32G32Sfloat;
        attributes[0].offset = offsetof(vertex, position);

        attributes[1].binding = 0;
        attributes[1].location = 1;
        attributes[1].format = vk::Format::eR32G32B32Sfloat;
        attributes[1].offset = offsetof(vertex, color);

        return attributes;
    }
}