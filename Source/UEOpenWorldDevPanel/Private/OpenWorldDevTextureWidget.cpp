#include "OpenWorldDevTextureWidget.h"

#include "OpenWorldDevWidget.h"
#include "OpenWorldHelper.h"
#include "UEOpenWorldDevPanelCommands.h"
#include "UEOpenWorldDevPanelStyle.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/SlateWrapperTypes.h"
#include "Engine/SceneCapture2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "VulkanRHI/Public/VulkanCommon.h"
#include "WorldPartition/WorldPartition.h"
#include "WorldPartition/WorldPartitionMiniMap.h"
#include "WorldPartition/WorldPartitionMiniMapHelper.h"
#include "WorldPartition/WorldPartitionMiniMapVolume.h"
#include "Fonts/FontMeasure.h"
#include "LevelEditorViewport.h"
#include "OpenWorldHelper.h"
#include "Styling/SlateStyleMacros.h"

SOpenWorldDevTextureWidget::SOpenWorldDevTextureWidget()
	:CommandList(MakeShareable(new FUICommandList))
{
	RootTreeItems.Empty();
	FOpenWorldHelper::ReadFromJsonFile(RootTreeItems, FOpenWorldHelper::Get()->GetSelectJsonPath());
}

void SOpenWorldDevTextureWidget::Construct(const FArguments& InArgs)
{

	// Defaults
	Trans = FVector2D(0, 0);
	Scale = 0.00133333332;
	TotalMouseDelta = 0;
	ChildSlot
	[
		SNew(SOverlay)

		// Top status bar
		+SOverlay::Slot()
		.VAlign(VAlign_Top)
		[
			SNew(SBorder)
#if ENGINE_MINOR_VERSION >= 1
			.BorderImage(FAppStyle::GetBrush(TEXT("Graph.TitleBackground")))
#else
			.BorderImage(FEditorStyle::GetBrush(TEXT("Graph.TitleBackground")))
#endif
			[
				SNew(SVerticalBox)

				+SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SHorizontalBox)

					+SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SButton)
						.Text(FText::FromString("LoadJson"))
						.OnClicked(this, &SOpenWorldDevTextureWidget::OnLoadJsonMap)
					]
					+SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SButton)
						.Text(FText::FromString("UpdateMap"))
						.OnClicked(this,&SOpenWorldDevTextureWidget::CaptureWorldTo2D)
					]
					+SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SButton)
						.Text(FText::FromString("SaveToImg"))
						.OnClicked(this,&SOpenWorldDevTextureWidget::CaptureWorldTo2D)
					]
				]
			]
		]

	];
	// BaseMapTexture 
	// Bind commands

	const FUEOpenWorldDevPanelCommands& Commands = FUEOpenWorldDevPanelCommands::Get();
	FUICommandList& ActionList = *CommandList;
	ActionList.MapAction(Commands.MoveCameraHere, FExecuteAction::CreateSP(this, &SOpenWorldDevTextureWidget::MoveCameraHere));
	ActionList.MapAction(Commands.DelecteSelected, FExecuteAction::CreateSP(this, &SOpenWorldDevTextureWidget::OnDeleteSelected));
	this->SetOnMouseMove(FPointerEventHandler::CreateSP(this,&SOpenWorldDevTextureWidget::OnMouseMove));
	this->SetOnMouseButtonDown(FPointerEventHandler::CreateSP(this,&SOpenWorldDevTextureWidget::OnMouseButtonDown));
	this->SetOnMouseButtonDown(FPointerEventHandler::CreateSP(this,&SOpenWorldDevTextureWidget::OnMouseButtonUp));
	FOpenWorldHelper::Get()->JsonDataChanged.AddRaw(this, &SOpenWorldDevTextureWidget::OnTreeDataChanged);

}

SOpenWorldDevTextureWidget::~SOpenWorldDevTextureWidget()
{
	FOpenWorldHelper::Get()->JsonDataChanged.RemoveAll(this);
}
FReply SOpenWorldDevTextureWidget::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	const bool bIsLeftMouseButtonEffecting = MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton;
	const bool bIsRightMouseButtonEffecting = MouseEvent.GetEffectingButton() == EKeys::RightMouseButton;
	const bool bIsMiddleMouseButtonEffecting = MouseEvent.GetEffectingButton() == EKeys::MiddleMouseButton;
	const bool bIsRightMouseButtonDown = MouseEvent.IsMouseButtonDown(EKeys::RightMouseButton);
	const bool bIsLeftMouseButtonDown = MouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton);
	const bool bIsMiddleMouseButtonDown = MouseEvent.IsMouseButtonDown(EKeys::MiddleMouseButton);

	TotalMouseDelta = 0;

	// if (bIsLeftMouseButtonEffecting || bIsMiddleMouseButtonEffecting || bIsRightMouseButtonEffecting)
	if (bIsLeftMouseButtonEffecting || bIsRightMouseButtonEffecting)
	{
		FReply ReplyState = FReply::Handled();
		ReplyState.CaptureMouse(SharedThis(this));

		if (bIsLeftMouseButtonEffecting)
		{
			if (!MouseEvent.IsControlDown())
			{
				// ClearSelection();
			}
			SelectionStart = MouseCursorPosWorld;
			SelectionEnd = SelectionStart;
			// SelectBox.Init();
			// SelectBoxGridSnapped.Init();
		
		}

		// if (bIsMiddleMouseButtonEffecting)
		// {
		// 	MeasureStart = MeasureEnd = MouseCursorPosWorld;
		// }

		return ReplyState;
	}

	return FReply::Handled();
}

FReply SOpenWorldDevTextureWidget::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	{
		const bool bIsLeftMouseButtonEffecting = MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton;
		const bool bIsRightMouseButtonEffecting = MouseEvent.GetEffectingButton() == EKeys::RightMouseButton;
		const bool bIsMiddleMouseButtonEffecting = MouseEvent.GetEffectingButton() == EKeys::MiddleMouseButton;
		const bool bIsRightMouseButtonDown = MouseEvent.IsMouseButtonDown(EKeys::RightMouseButton);
		const bool bIsLeftMouseButtonDown = MouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton);
		const bool bIsMiddleMouseButtonDown = MouseEvent.IsMouseButtonDown(EKeys::MiddleMouseButton);
	
		TotalMouseDelta = 0;
//		if (bIsLeftMouseButtonEffecting || bIsMiddleMouseButtonEffecting || bIsRightMouseButtonEffecting)
		if (bIsLeftMouseButtonEffecting || bIsRightMouseButtonEffecting)
		{
			FReply ReplyState = FReply::Handled();
	
			const bool bHasMouseCapture = bIsDragSelecting || bIsPanning || bIsMeasuring;
			MouseCursorPos = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
			MouseCursorPosWorld = ScreenToWorld.TransformPoint(MouseCursorPos);
	
			if (!bHasMouseCapture && bIsRightMouseButtonEffecting)
			{			
				// if (HoveredLoaderInterface.IsValid() && !SelectedLoaderInterfaces.Contains(HoveredLoaderInterface))
				// {
				// 	SelectedLoaderInterfaces.Reset();
				// 	SelectedLoaderInterfaces.Add(HoveredLoaderInterface);
				// }
			
				FMenuBuilder MenuBuilder(true, CommandList);
				
				const FUEOpenWorldDevPanelCommands& Commands = FUEOpenWorldDevPanelCommands::Get();
				
				// if (!GetDefault<FUEOpenWorldDevPanelCommands>())//->bDisableLoadingInEditor
				// {
				// 	MenuBuilder.BeginSection(NAME_None, LOCTEXT("WorldPartitionSelection", "Selection"));
				// 	MenuBuilder.AddMenuEntry(Commands.CreateRegionFromSelection);
				// 	MenuBuilder.AddMenuSeparator();
				// 	MenuBuilder.AddMenuEntry(Commands.LoadSelectedRegions);
				// 	MenuBuilder.AddMenuEntry(Commands.UnloadSelectedRegions);
				// 	MenuBuilder.AddMenuSeparator();
				// 	MenuBuilder.AddMenuEntry(Commands.ConvertSelectedRegionsToActors);
				// 	MenuBuilder.EndSection();
				// }
				
				MenuBuilder.BeginSection(NAME_None, FText::FromString("OpenWorldDevPannelMisc"));
				MenuBuilder.AddMenuEntry(Commands.MoveCameraHere);
				MenuBuilder.AddMenuEntry(Commands.DelecteSelected);
				// if (!GetDefault<UWorldPartitionEditorSettings>()->bDisablePIE)
				// {
				// 	MenuBuilder.AddMenuEntry(Commands.PlayFromHere);
				// }
				//
				// if (!GetDefault<UWorldPartitionEditorSettings>()->bDisableLoadingInEditor)
				// {
				// 	MenuBuilder.AddMenuEntry(Commands.LoadFromHere);
				// }
				MenuBuilder.EndSection();			

				FWidgetPath WidgetPath = MouseEvent.GetEventPath() != nullptr ? *MouseEvent.GetEventPath() : FWidgetPath();
				FSlateApplication::Get().PushMenu(AsShared(), WidgetPath, MenuBuilder.MakeWidget(), MouseEvent.GetScreenSpacePosition(), FPopupTransitionEffect(FPopupTransitionEffect::ContextMenu));

			}
	
			if (bIsLeftMouseButtonEffecting)
			{
				bIsDragSelecting = false;
			}
	
			if (bIsRightMouseButtonEffecting)
			{
				bIsPanning = false;
			}
	
			// if (bIsMiddleMouseButtonEffecting)
			// {
			// 	bIsMeasuring = false;
			// }
	
			if (HasMouseCapture() && !bIsDragSelecting && !bIsPanning && !bIsMeasuring)
			{
				ReplyState.ReleaseMouseCapture();
			}
	
			return ReplyState;
		}
	
		return FReply::Handled();
	}
}

void SOpenWorldDevTextureWidget::UpdateTransform() const
{
	FTransform2d T;
	FTransform2d V;

	//鼠标移动的transform 不 不包括scle
	T = FTransform2d(1.0f, Trans); 
	//放大缩小的transform 找到窗口的中心点
	V = FTransform2d(Scale, FVector2D(ScreenRect.GetSize().X * 0.5f, ScreenRect.GetSize().Y * 0.5f));

	//先放缩小之后再平移
	WorldToScreen = T.Concatenate(V);
	ScreenToWorld = WorldToScreen.Inverse();
}

int32 SOpenWorldDevTextureWidget::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	if (BaseMapTexture)
	{
		WorldMiniMapBounds = FBox2D(FVector2D(WorldBound.Min), FVector2D(WorldBound.Max));
		WorldMiniMapBrush.SetResourceObject(BaseMapTexture);
		WorldMiniMapBrush.SetUVRegion(FBox2D(FVector2D(0,0),FVector2D(1,1)));
		WorldMiniMapBrush.SetImageSize(FVector2D(BaseMapTexture->GetImportedSize().X,BaseMapTexture->GetImportedSize().Y));
		
		const bool bResetView = !ScreenRect.bIsValid;
	
		ScreenRect = FBox2D(FVector2D(0, 0), AllottedGeometry.GetLocalSize());
		// ScreenRect = FBox2D(FVector2D(0, 0),FVector2D(700, 700));
	
		UpdateTransform();
		LayerId = PaintMinimap(AllottedGeometry,MyCullingRect,OutDrawElements,LayerId);
		LayerId = PaintActors(AllottedGeometry,MyCullingRect,OutDrawElements,LayerId);
	}
	else //去指定路径中加载
	{
		CaptureWorldTo2D();
	}

	return SCompoundWidget::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
}

int32 SOpenWorldDevTextureWidget::PaintMinimap(const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId) const
{
	if (UTexture2D* Texture2D = Cast<UTexture2D>(WorldMiniMapBrush.GetResourceObject()))
	{
		const FBox2D MinimapBounds(
			WorldToScreen.TransformPoint(WorldMiniMapBounds.Min),
			WorldToScreen.TransformPoint(WorldMiniMapBounds.Max)
		);

		const FPaintGeometry WorldImageGeometry = AllottedGeometry.ToPaintGeometry(
			MinimapBounds.Min,
			MinimapBounds.Max - MinimapBounds.Min
		);

		
		FSlateDrawElement::MakeBox(
			OutDrawElements,
			LayerId-1,
			WorldImageGeometry,
			&WorldMiniMapBrush,
			ESlateDrawEffect::None,
			FLinearColor::White
		);
	}
	return  LayerId;
}

const FSlateBrush *GetBrushByIconType(EIconType IconType)
{
	switch (IconType)
	{
		case EIconType::Monster : return FUEOpenWorldDevPanelStyle::Get().GetBrush("OpenWorldDevPannle.Monster");break;;
		case EIconType::Boss : return FUEOpenWorldDevPanelStyle::Get().GetBrush("OpenWorldDevPannle.Boss"); break;
		case EIconType::ButtonIcon : return FUEOpenWorldDevPanelStyle::Get().GetBrush("OpenWorldDevPannle.Button"); break;
		case EIconType::Test1 : return FUEOpenWorldDevPanelStyle::Get().GetBrush("OpenWorldDevPannle.Boss");break;
		case EIconType::Test2 : return FUEOpenWorldDevPanelStyle::Get().GetBrush("OpenWorldDevPannle.Monster");break;

		default:return nullptr;
	}
}

int32 SOpenWorldDevTextureWidget::PaintActors(const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId) const
{
	const FBox2D MinimapBounds(
		WorldToScreen.TransformPoint(WorldMiniMapBounds.Min),
		WorldToScreen.TransformPoint(WorldMiniMapBounds.Max)
	);
	DisplayedItems.Empty();
	for(auto item : RootTreeItems)
	{
		if(item->CheckBoxState != ECheckBoxState::Unchecked)
		{
			for(auto child : item->GetChildren())
			{
				if(child->CheckBoxState == ECheckBoxState::Checked)
					for(auto pos : child->IconPostions)
					{
						const FSlateBrush *Brush = GetBrushByIconType(EIconType(child->IconType));

						FVector2d	WorldPositions = FVector2d(0.0);
						WorldPositions = FVector2d(pos.X,pos.Y);
						FVector2d RelativeNormalPosition = (WorldPositions - WorldMiniMapBounds.Min)/((WorldMiniMapBounds.Max - WorldMiniMapBounds.Min));
						FVector2d IconScreenSize = (MinimapBounds.Max - MinimapBounds.Min) * 0.01;
						// IconScreenSize.X =  FMath::Clamp(IconScreenSize.X,IconMinSize.X,IconMaxSize.X);
						// IconScreenSize.Y =  FMath::Clamp(IconScreenSize.Y,IconMinSize.Y,IconMaxSize.Y);
						FVector2d LocalOffset= MinimapBounds.Min + RelativeNormalPosition * (MinimapBounds.Max - MinimapBounds.Min) - IconScreenSize/2;
						FBox2d IconWorldBound(ScreenToWorld.TransformPoint(LocalOffset), ScreenToWorld.TransformPoint(LocalOffset + IconScreenSize));
						const FPaintGeometry WorldImageGeometry = AllottedGeometry.ToPaintGeometry(LocalOffset,IconScreenSize);
						DisplayedItems.Add(FDrawIconInfo(pos,WorldPositions,IconWorldBound,child));
						bool bSelected = false;
						for(auto SelectedItem : SelectedItems)
						{
							if(SelectedItem.ItemPtr == child && SelectedItem.WorldPosition == pos)
							{
								bSelected = true;
								break;;
							}
						}
						if(bSelected)
						{
							FSlateDrawElement::MakeBox(
								OutDrawElements,
								LayerId-1,
								WorldImageGeometry,
								Brush,
								ESlateDrawEffect::None,
								FLinearColor::Red);
						}
						else
						{
							FSlateDrawElement::MakeBox(
								OutDrawElements,
								LayerId-1,
								WorldImageGeometry,
								Brush,
								ESlateDrawEffect::None,
								FLinearColor::White);
						}
						
					}
			}
		}
	}
	//current mouse position
	FLinearColor BackgroundColor = FLinearColor::Black.CopyWithNewOpacity(0.35f);
	FText InstructionText = FText::FromString(FString::Format(TEXT("X: {0}\nY: {1}"),{MouseCursorPosWorld.X,MouseCursorPosWorld.Y}));
	const FSlateBrush*   WhiteBrush = FAppStyle::GetBrush("WhiteBrush");
	const TSharedRef<FSlateFontMeasure> FontMeasure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
	const FSlateFontInfo FontInfo = FCoreStyle::Get().GetFontStyle("FontAwesome.13");
	const FVector2D LabelSize = FontMeasure->Measure(InstructionText, FontInfo);
	const FVector2D GeometrySize = AllottedGeometry.GetLocalSize();
	const FVector2D LabelOffset = FVector2D((GeometrySize.X - LabelSize.X) / 2.f, (GeometrySize.Y - LabelSize.Y) / 2.f);
	const FPaintGeometry LabelGeometry = AllottedGeometry.ToPaintGeometry(FSlateLayoutTransform(MouseCursorPos));

	FSlateDrawElement::MakeText(OutDrawElements, LayerId, LabelGeometry, InstructionText, FontInfo);
	return  LayerId;
}
void SOpenWorldDevTextureWidget::CustomPaint(FString key, EIconType Icon, FVector2d WorldPositions)
{
	WorldPositions = FVector2d(0.0);
	FVector2d RelativeNormalPosition = WorldMiniMapBounds.Min + (WorldPositions - WorldMiniMapBounds.Min)/((WorldMiniMapBounds.Max - WorldMiniMapBounds.Min));
	
}
void SOpenWorldDevTextureWidget::CustomPaint(FString key, EIconType Icon, TArray<FVector2d> WorldPositions)
{
	
}

void SOpenWorldDevTextureWidget::MoveCameraHere()
{
	{
		FVector WorldLocation = FVector(MouseCursorPosWorld, 0);

		FHitResult HitResult;
		const FVector TraceStart(WorldLocation.X, WorldLocation.Y, HALF_WORLD_MAX);
		const FVector TraceEnd(WorldLocation.X, WorldLocation.Y, -HALF_WORLD_MAX);
		const FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(SWorldPartitionEditorGrid2D_MoveCameraHere), true);
		const bool bHitResultValid = GEditor->GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic, TraceParams);

		for (FLevelEditorViewportClient* LevelVC : GEditor->GetLevelViewportClients())
		{
			WorldLocation.Z = bHitResultValid ? HitResult.ImpactPoint.Z + 1000.0f : LevelVC->GetViewLocation().Z;

			LevelVC->SetViewLocation(WorldLocation);
			LevelVC->Invalidate();

			FEditorDelegates::OnEditorCameraMoved.Broadcast(WorldLocation, LevelVC->GetViewRotation(), LevelVC->ViewportType, LevelVC->ViewIndex);
		}
	}
}

void SOpenWorldDevTextureWidget::OnDeleteSelected()
{
	// for (auto SelectedItem : SelectedItems)
	// {
	// 	TArray<int32> DeletePositionsIndex; 
	// 	for(int Index = 0; Index < SelectedItem.ItemPtr->IconPostions.Num() ; Index++)
	// 	{
	// 		if(SelectedItem.ItemPtr->IconPostions[Index] == SelectedItem.WorldPosition)
	// 		{
	// 			DeletePositionsIndex.Add(Index);
	// 		}
	// 	}
	// 	while (!DeletePositionsIndex.IsEmpty())
	// 	{
	// 		SelectedItem.ItemPtr->IconPostions.RemoveAt(DeletePositionsIndex[DeletePositionsIndex.Num()-1]);
	// 		DeletePositionsIndex.Pop();
	// 	}
	// }
	
}

FReply SOpenWorldDevTextureWidget::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	
	const FVector2D CursorDelta = MouseEvent.GetCursorDelta();
	MouseCursorPos = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
	MouseCursorPosWorld = ScreenToWorld.TransformPoint(MouseCursorPos);
	MouseCurentPosition = MouseCursorPosWorld;
	GetSelectedItems(MouseCursorPosWorld);
	if (HasMouseCapture())
	{
		TotalMouseDelta += CursorDelta.Size();

		const bool bIsRightMouseButtonDown = MouseEvent.IsMouseButtonDown(EKeys::RightMouseButton);
		const bool bIsLeftMouseButtonDown = MouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton);
		const bool bIsMiddleMouseButtonDown = MouseEvent.IsMouseButtonDown(EKeys::MiddleMouseButton);
		// const bool bIsDragTrigger = IsInteractive() && (TotalMouseDelta > FSlateApplication::Get().GetDragTriggerDistance());

		// if (bIsMiddleMouseButtonDown)
		// {
		// 	if (!bIsMeasuring)
		// 	{
		// 		bIsMeasuring = true;
		// 	}
		//
		// 	if (bIsMeasuring)
		// 	{
		// 		MeasureEnd = MouseCursorPosWorld;
		// 	}
		// }

		if (bIsLeftMouseButtonDown)
		{
			if (!bIsDragSelecting)
			{
				bIsDragSelecting = true;
			}

			if (bIsDragSelecting)
			{
				SelectionEnd = MouseCursorPosWorld;
				// UpdateSelectionBox(MouseEvent.IsShiftDown());
				return FReply::Handled();
			}
		}

		if (bIsRightMouseButtonDown && !bIsDragSelecting)
		{
			if (!bIsPanning)
			{
				bIsPanning = true;
				LastMouseCursorPosWorldDrag = MouseCursorPosWorld;
			}

			if (bIsPanning)
			{
				Trans += (MouseCursorPosWorld - LastMouseCursorPosWorldDrag);
				UpdateTransform();
				return FReply::Handled();
			}
		}
	}

	return FReply::Handled();
}


UPackage* SOpenWorldDevTextureWidget::CreateNewPackage(FString AssetPath,FString FileName) const
{
	// FString AssetPath = TEXT("/Game/");
	AssetPath += FileName;
	UPackage* Package = FindPackage(NULL, *AssetPath);
	if(Package == nullptr)
	{
		Package = CreatePackage(NULL, *AssetPath);
	}
	Package->FullyLoad();
	return Package;
}

void SOpenWorldDevTextureWidget::CalTopViewOfWorld(FMatrix& OutProjectionMatrix, const FBox& WorldBox, uint32 ViewportWidth, uint32 ViewportHeight) const
{
	const FVector Origin = WorldBox.GetCenter();

	FVector2D WorldSizeMin2D(WorldBox.Min.X, WorldBox.Min.Y);
	FVector2D WorldSizeMax2D(WorldBox.Max.X, WorldBox.Max.Y);

	FVector2D WorldSize2D = (WorldSizeMax2D - WorldSizeMin2D);
	WorldSize2D.X = FMath::Abs(WorldSize2D.X);
	WorldSize2D.Y = FMath::Abs(WorldSize2D.Y);
	const bool bUseXAxis = (WorldSize2D.X / WorldSize2D.Y) > 1.f;
	const float WorldAxisSize = bUseXAxis ? WorldSize2D.X : WorldSize2D.Y;
	const uint32 ViewportAxisSize = bUseXAxis ? ViewportWidth : ViewportHeight;
	const float OrthoZoom = WorldAxisSize / ViewportAxisSize / 2.f;
	const float OrthoWidth = FMath::Max(1.f, ViewportWidth * OrthoZoom);
	const float OrthoHeight = FMath::Max(1.f, ViewportHeight * OrthoZoom);

#if ENGINE_MINOR_VERSION >= 1
	const float ZOffset = UE_FLOAT_HUGE_DISTANCE / 2;
#else
	const float ZOffset = WORLDPARTITION_MAX * 0.5;
#endif
	OutProjectionMatrix = FReversedZOrthoMatrix(
		OrthoWidth,
		OrthoHeight,
		0.5f / ZOffset,
		ZOffset
	);

	ensureMsgf(!OutProjectionMatrix.ContainsNaN(), TEXT("Nans found on ProjectionMatrix"));
	if (OutProjectionMatrix.ContainsNaN())
	{
		OutProjectionMatrix.SetIdentity();
	}
}

FReply SOpenWorldDevTextureWidget::OnLoadJsonMap() const
{
	FOpenWorldHelper::SelectMapJson();
	
	return FReply::Handled();
}

FReply SOpenWorldDevTextureWidget::CaptureWorldTo2D() const
{
	UWorld *EditorWorld = GEditor->GetEditorWorldContext().World();
	/*
	 * 获取worldBounds
	 */
	if((!EditorWorld || !EditorWorld->GetWorldPartition() || !EditorWorld->GetWorldPartition()->EditorHash))
	{
		return FReply::Handled();
	}
	WorldBound = EditorWorld->GetWorldPartition()->GetEditorWorldBounds();
	
	FBox2D WorldBounds2D(FVector2D(WorldBound.Min), FVector2D(WorldBound.Max));
	FVector2D ViewSize = WorldBounds2D.Max - WorldBounds2D.Min;
	float AspectRatio = FMath::Abs(ViewSize.X) / FMath::Abs(ViewSize.Y);
	uint32 ViewportWidth = 2048 * AspectRatio;
	uint32 ViewportHeight = 2048;

	FActorSpawnParameters SpawnInfo;
	SpawnInfo.ObjectFlags |= RF_Transient;
	//Calculate Projection matrix based on world bounds.
	FMatrix ProjectionMatrix;
	CalTopViewOfWorld(ProjectionMatrix, WorldBound, ViewportWidth, ViewportHeight);
	FVector CaptureActorLocation = FVector(WorldBound.GetCenter().X, WorldBound.GetCenter().Y, WorldBound.GetCenter().Z + WorldBound.GetExtent().Z);
	FRotator CaptureActorRotation = FRotator(-90.f, 0.f, -90.f);	
	if(TextureTarget != nullptr && ( TextureTarget->SizeX != ViewportWidth || TextureTarget->SizeY != ViewportHeight ))
	{
		TextureTarget->ReleaseResource();
		TextureTarget = nullptr;
		if(BaseMapTexture)
		{
			BaseMapTexture->ReleaseResource();
			BaseMapTexture = nullptr;
		}
	}
	if(TextureTarget == nullptr)
	{
		TextureTarget= NewObject<UTextureRenderTarget2D>();
		TextureTarget->ClearColor = FLinearColor(0.5,0.5,0.5,1);
		TextureTarget->TargetGamma = 2.2f;
		TextureTarget->InitCustomFormat(ViewportWidth, ViewportHeight, PF_B8G8R8A8, false);
		TextureTarget->UpdateResourceImmediate(true);
	}
	if( SceneCapture2DActor == nullptr)
	{
		SceneCapture2DActor = EditorWorld->SpawnActor<ASceneCapture2D>(CaptureActorLocation, CaptureActorRotation,SpawnInfo);
	}
	if(SceneCapture2DActor)
	{
		if(SceneCapture2DActor->GetCaptureComponent2D())
		{
			SceneCapture2DActor->GetCaptureComponent2D()->ProjectionType = ECameraProjectionMode::Orthographic;
			SceneCapture2DActor->GetCaptureComponent2D()->OrthoWidth = ViewportWidth;
			SceneCapture2DActor->GetCaptureComponent2D()->TextureTarget = TextureTarget;
			SceneCapture2DActor->GetCaptureComponent2D()->bCaptureEveryFrame = false;
			SceneCapture2DActor->GetCaptureComponent2D()->bCaptureOnMovement = false;
			SceneCapture2DActor->GetCaptureComponent2D()->CaptureSource = SCS_FinalToneCurveHDR;
			SceneCapture2DActor->GetCaptureComponent2D()->bUseCustomProjectionMatrix = true;
			SceneCapture2DActor->GetCaptureComponent2D()->CustomProjectionMatrix = ProjectionMatrix;
			SceneCapture2DActor->GetCaptureComponent2D()->CaptureScene();
		}
		else
		{
			SceneCapture2DActor->Destroy();
			SceneCapture2DActor = nullptr;
			SceneCapture2DActor = EditorWorld->SpawnActor<ASceneCapture2D>(CaptureActorLocation, CaptureActorRotation,SpawnInfo);
		}
	}
	// SceneCapture2DActor->Destroy();
	// SceneCapture2DActor = nullptr;
	TArray<FStringFormatArg> args = {FString::FromInt(ViewportWidth),FString::FromInt(ViewportWidth)};
	FString FileName = FString::Format(TEXT("BaseMap_{0}x{1}"),args);
	if(BaseMapTexture == nullptr)
	{
		UPackage *Package = CreateNewPackage("/Game/",FileName);

		BaseMapTexture = NewObject<UTexture2D>(Package,*FileName,RF_Public | RF_Standalone | RF_MarkAsRootSet);
		BaseMapTexture->MipGenSettings = TextureMipGenSettings::TMGS_NoMipmaps;
		TextureTarget->UpdateTexture2D(BaseMapTexture,TextureTarget->GetTextureFormatForConversionToTexture2D(),  TextureTarget->GetMaskedFlags());
		
		// NewTexture2D->Source.Init(MiniMapTexture2D->GetSizeX(), MiniMapTexture2D->GetSizeY(), 1, 1, ETextureSourceFormat::TSF_BGRA8, Pixels);
    	BaseMapTexture->UpdateResource();
    	Package->MarkPackageDirty();
    	FAssetRegistryModule::AssetCreated(BaseMapTexture);
    	//通过asset路径获取包中文件名
    	FString PackageFileName = FPackageName::LongPackageNameToFilename("/Game/"+FileName, FPackageName::GetAssetPackageExtension());
    	//进行保存
    	bool bSaved = UPackage::SavePackage(Package, BaseMapTexture, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone, *PackageFileName, GError, nullptr, true, true, SAVE_NoError);
	}
	else
	{
		TextureTarget->UpdateTexture2D(BaseMapTexture,TextureTarget->GetTextureFormatForConversionToTexture2D(),  TextureTarget->GetMaskedFlags());
		UPackage *Package = CreateNewPackage();
		Package->MarkPackageDirty();
		FString PackageFileName = FPackageName::LongPackageNameToFilename("/Game/" + FileName, FPackageName::GetAssetPackageExtension());
		//进行保存
		bool bSaved = UPackage::SavePackage(Package, BaseMapTexture, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone, *PackageFileName, GError, nullptr, true, true, SAVE_NoError);
	}
	return FReply::Handled();
}

//既要放大缩小也要平移(先移动到中心点->放大缩小->再移动回去)
FReply SOpenWorldDevTextureWidget::OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	FTransform2d(1.0f, Trans); 
	//放大缩小的transform 找到窗口的中心点
	FVector2D MousePosLocalSpace = MouseCursorPos - MyGeometry.GetLocalSize() * 0.5f;
	UE_LOG(LogTemp, Warning, TEXT("MyGeometry.GetLocalSize(): %f , %f"), MyGeometry.GetLocalSize().X,MyGeometry.GetLocalSize().Y);
	FVector2D P0 = MousePosLocalSpace / Scale;
	float Delta = 1.0f + FMath::Abs(MouseEvent.GetWheelDelta() / 4.0f);
	Scale = FMath::Clamp(Scale * (MouseEvent.GetWheelDelta() > 0 ? Delta : (1.0f / Delta)), 0.00000001f, 10.0f);
	FVector2D P1 = MousePosLocalSpace / Scale;
	Trans += (P1 - P0);
	UpdateTransform();
	return FReply::Handled();
	
}


FReply SOpenWorldDevTextureWidget::OnMouseButtonDoubleClick(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent)
{
	MoveCameraHere();
	return FReply::Handled();
}

void SOpenWorldDevTextureWidget::OnTreeDataChanged(FString InTag)
{
	if (InTag == "Tree")
	{
		RootTreeItems.Empty();
		FOpenWorldHelper::ReadFromJsonFile(RootTreeItems, FOpenWorldHelper::Get()->GetSelectJsonPath());
	}
}

void SOpenWorldDevTextureWidget::GetSelectedItems(FVector2D InMouseCursorPosWorld)
{
	SelectedItems.Empty();
	for(auto DisplayedItem : DisplayedItems)
	{
		FVector2d BoundMax = DisplayedItem.WorldIconBound.Max;
		FVector2d BoundMin = DisplayedItem.WorldIconBound.Min;
		if(InMouseCursorPosWorld >= BoundMin && InMouseCursorPosWorld <= BoundMax)
		{
			SelectedItems.Add(DisplayedItem);
		}
	}
}
