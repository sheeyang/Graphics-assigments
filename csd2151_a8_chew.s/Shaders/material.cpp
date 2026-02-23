/*!*****************************************************************************
\file CG/material.cpp
\author Vadim Surov (vsurov@digipen.edu)
\date 12/23/2025 (MM/DD/YYYY)
\brief Material implementation: stores named float parameter arrays and uploads
       them as uniforms to shader programs.
*******************************************************************************/
#include "material.h"

namespace cg
{
    // Constructor initializes the material parameters using an initializer list
    Material::Material(const std::initializer_list< std::pair<std::string, std::initializer_list<float>> >& params)
        : params{ }
    {
        // Copy parameters into the internal map
        for (const std::pair < std::string, std::initializer_list<float>>& p : params)
            this->params.insert(p);
    }

    // Set the material uniforms in a shader program
    void Material::setUniforms(Program* pProgram) const
    { 
        // Loop over each parameter and set it in the shader program
        for (const std::pair <std::string, std::vector<float>>& p : params)
            pProgram->setUniform(p.first, p.second); // Set uniform by name and value
    }
}
