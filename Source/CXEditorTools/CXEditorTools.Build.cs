using UnrealBuildTool;
public class CXEditorTools : ModuleRules
{
 public CXEditorTools(ReadOnlyTargetRules Target) : base(Target)
 {
  PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
  PrivateDependencyModuleNames.AddRange(new string[]{"Core","CoreUObject","Engine","CX","UMG","UMGEditor","UnrealEd","Kismet","BlueprintGraph","KismetCompiler","SlateCore","EnhancedInput","InputCore"});
 }
}
