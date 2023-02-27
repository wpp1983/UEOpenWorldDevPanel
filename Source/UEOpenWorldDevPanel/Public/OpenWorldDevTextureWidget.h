#pragma once
#include "OpenWorldHelper.h"
#include "Engine/SceneCapture2D.h"

UENUM(BlueprintType)
enum EIconType
{
	Monster UMETA(DisplayName = "Weather Preset"),
	Boss UMETA(DisplayName = "Weather Settings"),
	ButtonIcon,
	Test1,
	Test2
};

struct FDrawIconInfo : public TSharedFromThis<FDrawIconInfo>
{
public:
	FDrawIconInfo(FVector Pos = FVector(0), FVector2d InScreenPosition = FVector2d(0), FBox2d InWorldIconBound = FBox2d(), TSharedPtr<FOpenWorldTreeItem> Item = nullptr)
		:WorldPosition(Pos),ScreenPosition(InScreenPosition),WorldIconBound(InWorldIconBound),ItemPtr(Item)
	{
	};
	~FDrawIconInfo(){};

	FVector WorldPosition;
	FVector2d ScreenPosition;
	FBox2d WorldIconBound;
	TSharedPtr<FOpenWorldTreeItem> ItemPtr;
};

class SOpenWorldDevTextureWidget :public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SOpenWorldDevTextureWidget)
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
	SOpenWorldDevTextureWidget();
	~SOpenWorldDevTextureWidget();
	
    virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
    virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonDoubleClick(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent) override;
	void MoveCameraHere();
	void OnDeleteSelected();
	FReply CaptureWorldTo2D() const;
	void OnTreeDataChanged(FString InTag);
protected:
	
	// virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	virtual int32 PaintMinimap(const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId) const;
	virtual int32 PaintActors(const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId) const;
	virtual void CustomPaint(FString key, EIconType Icon, TArray<FVector2d> WorldPositions);
	virtual void CustomPaint(FString key, EIconType Icon, FVector2d WorldPositions);
	void UpdateTransform()const;
	void CalTopViewOfWorld(FMatrix& OutProjectionMatrix, const FBox& WorldBox, uint32 ViewportWidth, uint32 ViewportHeight) const;
	void GetSelectedItems(FVector2D InMouseCursorPosWorld);
	
	UPackage* CreateNewPackage(FString AssetPath = "/Game/",FString FileName = "BaseMapTexture") const;

	const TSharedRef<FUICommandList> CommandList;
	
	mutable TObjectPtr<class UTextureRenderTarget2D> TextureTarget = nullptr;
	mutable UTexture2D *BaseMapTexture = nullptr;
	mutable ASceneCapture2D* SceneCapture2DActor = nullptr;
	mutable FSlateBrush WorldMiniMapBrush;
	mutable UTexture2D *MiniMapTexture = nullptr;
	mutable FBox2D ScreenRect;
	mutable FBox2D WorldMiniMapBounds;
	mutable FTransform2d WorldToScreen;
	mutable FTransform2d ScreenToWorld;
	mutable float Scale;
	mutable FVector2D Trans;
	mutable  FBox WorldBound;
	float TotalMouseDelta;
	FVector2D MouseCursorPos;
	FVector2D MouseCursorPosWorld;
	FVector2D LastMouseCursorPosWorldDrag;
	FVector2D SelectionStart;
	FVector2D SelectionEnd;
	FVector2D MeasureStart;
	FVector2D MeasureEnd;
	FVector2D MouseCurentPosition;
	bool bIsDragSelecting;
	bool bIsPanning;
	bool bIsMeasuring = false;
	bool bShowActors;

	FVector2d IconMinSize = FVector2d(5);
	FVector2d IconMaxSize = FVector2d(10);
	TArray<TSharedPtr<FOpenWorldTreeItem>> RootTreeItems;
	TArray<FDrawIconInfo> SelectedItems;
	mutable  TArray<FDrawIconInfo> DisplayedItems;

};
