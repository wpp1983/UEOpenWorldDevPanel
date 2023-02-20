#include "OpenWorldDevWidget.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/SceneCapture2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "WorldPartition/WorldPartition.h"
#include "WorldPartition/WorldPartitionMiniMap.h"
#include "WorldPartition/WorldPartitionMiniMapHelper.h"

SOpenWorldDevWidget::SOpenWorldDevWidget()
{
	
}

void SOpenWorldDevWidget::Construct(const FArguments& InArgs)
{
ChildSlot
	[
		SNew(SOverlay)

		// Top status bar
		+SOverlay::Slot()
		.VAlign(VAlign_Top)
		[
			SNew(SBorder)
			.BorderImage(FEditorStyle::GetBrush(TEXT("Graph.TitleBackground")))
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
						.Text(FText::FromString("Capture World To 2DTextrue"))
						.OnClicked(this,&SOpenWorldDevWidget::CaptureWorldTo2D)
					]
					// +SHorizontalBox::Slot()
					// .FillWidth(1.0f)
					// .VAlign(VAlign_Center)
					// [
					// 	SNew(STextBlock)
					// 	.AutoWrapText(true)
					// 	.IsEnabled(true)
					// 	.Text(LOCTEXT("ShowActors", "Show Actors"))
					// ]
					// +SHorizontalBox::Slot()
					// .AutoWidth()
					// [
					// 	SNew(SCheckBox)
					// 	.IsChecked(GetMutableDefault<UWorldPartitionEditorPerProjectUserSettings>()->GetBugItGoLoadCells() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
					// 	.IsEnabled(true)
					// 	.OnCheckStateChanged(FOnCheckStateChanged::CreateLambda([=](ECheckBoxState State) { GetMutableDefault<UWorldPartitionEditorPerProjectUserSettings>()->SetBugItGoLoadCells(State == ECheckBoxState::Checked); }))
					// ]
					// +SHorizontalBox::Slot()
					// .FillWidth(1.0f)
					// .VAlign(VAlign_Center)
					// [
					// 	SNew(STextBlock)
					// 	.AutoWrapText(true)
					// 	.IsEnabled(true)
					// 	.Text(LOCTEXT("BugItGoLoadCells", "BugItGo Load Cells"))
					// ]
					// +SHorizontalBox::Slot()
					// .AutoWidth()
					// [
					// 	SNew(SCheckBox)
					// 	.IsChecked(GetMutableDefault<UWorldPartitionEditorPerProjectUserSettings>()->GetShowCellCoords() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
					// 	.IsEnabled(true)
					// 	.OnCheckStateChanged(FOnCheckStateChanged::CreateLambda([=](ECheckBoxState State) { GetMutableDefault<UWorldPartitionEditorPerProjectUserSettings>()->SetShowCellCoords(State == ECheckBoxState::Checked); }))
					// ]
					// +SHorizontalBox::Slot()
					// .FillWidth(1.0f)
					// .VAlign(VAlign_Center)
					// [
					// 	SNew(STextBlock)
					// 	.AutoWrapText(true)
					// 	.IsEnabled(true)
					// 	.Text(LOCTEXT("ShowCellCoords", "Show Cell Coords"))
					// ]
					// + SHorizontalBox::Slot()
					// .AutoWidth()
					// [
					// 	SNew(SButton)
					// 	.Text(LOCTEXT("FocusSelection", "Focus Selection"))
					// 	.OnClicked(this, &SWorldPartitionEditorGrid2D::FocusSelection)
					// ]
				]
			]
		]

	];

	// Bind commands

	this->SetOnMouseMove(InArgs._OnMouseMove);
}
SOpenWorldDevWidget::~SOpenWorldDevWidget()
{
	
}


void SOpenWorldDevWidget::UpdateTransform() const
{
	FTransform2D T;
	FTransform2D V;
	T = FTransform2D(1.0f, Trans);
	// V = FTransform2D(Scale, FVector2D(ScreenRect.GetSize().X * 0.5f, ScreenRect.GetSize().Y * 0.5f));
	V = FTransform2D(0.05, FVector2D(ScreenRect.GetSize().X * 0.5f, ScreenRect.GetSize().Y * 0.5f));

	WorldToScreen = T.Concatenate(V);
	ScreenToWorld = WorldToScreen.Inverse();
}

FReply SOpenWorldDevWidget::UpdateWorldMiniMapDetails() const
{
	UWorld *EditorWorld = GEditor->GetEditorWorldContext().World();
	AWorldPartitionMiniMap* WorldMiniMap = FWorldPartitionMiniMapHelper::GetWorldPartitionMiniMap(EditorWorld);
	if (WorldMiniMap)
	{
		WorldMiniMapBounds = FBox2D(FVector2D(WorldMiniMap->MiniMapWorldBounds.Min), FVector2D(WorldMiniMap->MiniMapWorldBounds.Max));
		MiniMapTexture = WorldMiniMap->MiniMapTexture.Get();
		if(TextureTarget)
		{
			WorldMiniMapBrush.SetResourceObject(TextureTarget);
		}
		else if (MiniMapTexture)
		{
			FString TexturName = "MiniMap";
			FBox MiniMapBound;
			FWorldPartitionMiniMapHelper::CaptureWorldMiniMapToTexture(EditorWorld,nullptr,1024,MiniMapTexture,TexturName,MiniMapBound);
			WorldMiniMapBrush.SetUVRegion(WorldMiniMap->UVOffset);
			WorldMiniMapBrush.SetImageSize(MiniMapTexture->GetImportedSize());
			WorldMiniMapBrush.SetResourceObject(MiniMapTexture);
		}
	}
	return FReply::Handled();
}

int32 SOpenWorldDevWidget::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	// FWeightedMovingAverageScope ProfileMeanValue(PaintTime);

	// UpdateWorldMiniMapDetails();
	if (MiniMapTexture)
	{
		const bool bResetView = !ScreenRect.bIsValid;
	
		ScreenRect = FBox2D(FVector2D(0, 0), AllottedGeometry.GetLocalSize());
		ScreenRect = FBox2D(FVector2D(0, 0),FVector2D(700, 700));
	
	
		UpdateTransform();
		LayerId = PaintMinimap(AllottedGeometry,MyCullingRect,OutDrawElements,LayerId);
		
	}

	return SCompoundWidget::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
}

int32 SOpenWorldDevWidget::PaintMinimap(const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId) const
{
	if (UTexture2D* Texture2D = Cast<UTexture2D>(WorldMiniMapBrush.GetResourceObject()))
	{
		// const FBox2D MinimapBounds(
		// 	WorldToScreen.TransformPoint(WorldMiniMapBounds.Min),
		// 	WorldToScreen.TransformPoint(WorldMiniMapBounds.Max)
		// );
		//
		// const FPaintGeometry WorldImageGeometry = AllottedGeometry.ToPaintGeometry(
		// 	MinimapBounds.Min,
		// 	MinimapBounds.Max - MinimapBounds.Min
		// );
		
		FSlateDrawElement::MakeBox(
			OutDrawElements,
			LayerId+1,
			AllottedGeometry.ToPaintGeometry(FVector2D(-512, -512), FVector2D(1024, 1024)),
			&WorldMiniMapBrush,
			ESlateDrawEffect::None,
			FLinearColor::Red
		);
	}
	return  LayerId;
}

FReply SOpenWorldDevWidget::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	UE_LOG(LogTemp, Warning, TEXT("mouse moving %f %f"),MyGeometry.Position.X,MyGeometry.Position.Y);
	return FReply::Handled();
}


UPackage* SOpenWorldDevWidget::CreateNewPackage(FString AssetPath,FString FileName)
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

void SOpenWorldDevWidget::CalTopViewOfWorld(FMatrix& OutProjectionMatrix, const FBox& WorldBox, uint32 ViewportWidth, uint32 ViewportHeight)
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

	const float ZOffset = WORLDPARTITION_MAX * 0.5;
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


FReply SOpenWorldDevWidget::CaptureWorldTo2D()
{
	UWorld *EditorWorld = GEditor->GetEditorWorldContext().World();
	/*
	 * 获取worldBounds
	 */
	FBox WorldBound = EditorWorld->GetWorldPartition()->GetEditorWorldBounds();
	
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


