#pragma once


class SOpenWorldDevWidget :public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SOpenWorldDevWidget)
	{}
	SLATE_EVENT(FPointerEventHandler, OnMouseMove)
	// /** Min Bounds for the View (in Seconds) */
	// SLATE_ATTRIBUTE(float, ViewMinInput)
	//
	// /** Max Bounds for the View (in Seconds) */
	// SLATE_ATTRIBUTE(float, ViewMaxInput)
	//
	// /** Should the control be able to edit this curve or is editing disabled? */
	// SLATE_ATTRIBUTE(bool, IsEditingEnabled)
	SLATE_END_ARGS()
public:
	void Construct(const FArguments& InArgs);
	SOpenWorldDevWidget();
	~SOpenWorldDevWidget();

protected:
	TSharedPtr<class SOpenWorldTreeWidget> OpenWorldDevTreeWidget;
	TSharedPtr<class SOpenWorldDevTextureWidget> OpenWorldDevTextureWidget;
};
