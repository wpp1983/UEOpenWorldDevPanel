#pragma once
#include "Engine/SceneCapture2D.h"

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
	
    virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

    virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
    protected:
	mutable FSlateBrush WorldMiniMapBrush;
	mutable UTexture2D *MiniMapTexture = nullptr;
	mutable FBox2D ScreenRect;
	mutable FBox2D WorldMiniMapBounds;
	mutable FVector2D Trans;
	// typedef TTransform2<float> FTransform2f;
	// typedef TTransform2<double> FTransform2d;
	mutable FTransform2D WorldToScreen;
	mutable FTransform2D ScreenToWorld;
	// virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	virtual int32 PaintMinimap(const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId) const;
	void UpdateTransform()const;


	
	UPackage* CreateNewPackage(FString AssetPath = "/Game/",FString FileName = "BaseMapTexture");
	void CalTopViewOfWorld(FMatrix& OutProjectionMatrix, const FBox& WorldBox, uint32 ViewportWidth, uint32 ViewportHeight);
	FReply CaptureWorldTo2D();


	FReply UpdateWorldMiniMapDetails() const;
	TSharedPtr<SImage> MiniMapImg;
	
	TObjectPtr<class UTextureRenderTarget2D> TextureTarget = nullptr;
	UTexture2D *BaseMapTexture = nullptr;
	ASceneCapture2D* SceneCapture2DActor = nullptr;
};
