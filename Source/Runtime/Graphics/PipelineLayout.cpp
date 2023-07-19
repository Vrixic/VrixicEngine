/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel)
* See "LICENSE.txt" for license information.
*/

#include "PipelineLayout.h"

uint8 PipelineLayout::BINDLESS_TEXTURE_BINDING_INDEX = 10u;
uint8 PipelineLayout::BINDLESS_TEXTURE_DESCRIPTOR_INDEX = 1u;
uint16 PipelineLayout::MAX_NUM_BINDLESS_RESOURCES = 1024u;

PipelineLayout::PipelineLayout() { }

