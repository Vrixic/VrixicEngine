#pragma once
#include "VulkanDevice.h"

/**
* Vulkan render layout used for RenderPass creation 
*/
class VulkanRenderLayout
{
private:
	VulkanDevice* Device;

protected:
	//uint32 NumAttachments;
	std::vector<VkAttachmentDescription> Attachments;

	uint32 NumColorAttachments;

	VkAttachmentReference ColorReference;
	VkAttachmentReference DepthReference;

	//uint32 NumInputAttachments;
	std::vector<VkAttachmentReference> InputAttachments;

	//uint32 NumPreserveAttachments;
	std::vector<uint32> PreserveAttachments;

	std::vector<VkAttachmentReference> ResolveAttachments;

	//uint32 NumClearValues;
	std::vector<VkClearValue> ClearValues;

	VkExtent2D Extent2D;
	//VkExtent3D Extent3D;

	VkRect2D RenderArea;

public:
	/**
	* @Param inNumColorAttachments - Number of color attachments for this render layout
	* @Param inRenderArea - RenderArea of this render layout
	* @Param inExtent2D (Optional) - The extent of this render layout
	*/
	VulkanRenderLayout(VulkanDevice* inDevice, uint32 inNumColorAttachments, VkRect2D& inRenderArea, VkExtent2D* inExtent2D = nullptr);
	~VulkanRenderLayout();

	//VulkanRenderLayout(const VulkanRenderLayout& other) = delete;
	VulkanRenderLayout operator=(const VulkanRenderLayout& other) = delete;

public:
	/**
	* Set attachments 
	*/
	void SetAttachments(std::vector<VkAttachmentDescription>& inAttachments);

	/**
	* Set input attachments 
	*/
	void SetInputAttachments(std::vector<VkAttachmentReference>& inAttachments);

	/**
	* Set preserve attachments
	*/
	void SetPreserveAttachments(std::vector<uint32>& inAttachments);

	/**
	* Set resolve attachments
	*/
	void SetResolveAttachments(std::vector<VkAttachmentReference>& inAttachments);

	/**
	* Set clear values 
	*/
	void SetClearValues(std::vector<VkClearValue>& inClearValues);

	/**
	* Set color reference 
	*/
	void SetColorReference(VkAttachmentReference inColorReference);

	/**
	* Set depth reference 
	*/
	void SetDepthReference(VkAttachmentReference inDepthReference);

	/**
	* Set render area 
	*/
	void SetRenderArea(VkRect2D& inRenderArea);

	/**
	* Set the extent
	*/
	void SetExtent2D(VkExtent2D& inExtent2D);

public:
	inline const uint32 GetNumAttachments() const
	{
		return (uint32)Attachments.size();
	}

	inline const uint32 GetNumColorAttachments() const
	{
		return (uint32)NumColorAttachments;
	}

	inline const uint32 GetNumInputAttachments() const
	{
		return (uint32)InputAttachments.size();
	}

	inline const uint32 GetNumPreserveAttachments() const
	{
		return (uint32)PreserveAttachments.size();
	}

	inline const uint32 GetNumClearValues() const
	{
		return (uint32)ClearValues.size();
	}

	inline const VkAttachmentDescription* GetAttachments() const
	{
		return Attachments.size() > 0 ? Attachments.data() : nullptr;
	}

	inline const VkAttachmentReference* GetInputAttachments() const
	{
		return InputAttachments.size() > 0 ? InputAttachments.data() : nullptr;
	}

	inline const uint32* GetPreserveAttachments() const
	{
		return PreserveAttachments.size() > 0 ? PreserveAttachments.data() : nullptr;
	}

	inline const VkAttachmentReference* GetResolveAttachments() const
	{
		return ResolveAttachments.size() > 0 ? ResolveAttachments.data() : nullptr;
	}

	inline const VkClearValue* GetClearValues() const
	{
		return ClearValues.size() > 0 ? ClearValues.data() : nullptr;
	}

	inline const VkAttachmentReference* GetColorReference() const
	{
		return &ColorReference;
	}

	inline const VkAttachmentReference* GetDepthReference() const
	{
		return &DepthReference;
	}

	inline const VkExtent2D* GetExtent2D() const
	{
		return &Extent2D;
	}

	//inline const VkExtent3D* GetExtent3D() const
	//{
	//	return &Extent3D;
	//}
	
	inline const VkRect2D* GetRenderArea() const
	{
		return &RenderArea;
	}

};


