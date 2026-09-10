using System;
using System.IO;
using Microsoft.UI.Dispatching;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Markup;
using Microsoft.UI.Xaml.Media;
using WinUI.Composition.Hlsl;

namespace HlslCSharpConsumer;

public partial class App : Application
{
    private Window? window;
    private DispatcherQueueTimer? timer;
    private Border? border;
    private int phase;
    public App() { InitializeComponent(); UnhandledException += (_, args) => { File.AppendAllText("csharp-results.txt", "FAIL " + args.Exception.ToString() + "\n"); }; }
    protected override void OnLaunched(LaunchActivatedEventArgs args)
    {
        File.WriteAllText("csharp-results.txt", "Started\n");
        try
        {
            var parsed = (Border)XamlReader.Load("""<Border xmlns="http://schemas.microsoft.com/winfx/2006/xaml/presentation" xmlns:hlsl="using:WinUI.Composition.Hlsl"><Border.Background><hlsl:LiquidGlassBrush IsEnabled="False" BlurRadius="10" RefractionStrength="16"/></Border.Background></Border>""");
            if (parsed.Background is not LiquidGlassBrush literal || literal.BlurRadius != 10 || literal.RefractionStrength != 16) throw new Exception("Numeric XAML parsing failed.");
            File.AppendAllText("csharp-results.txt", "PASS: numeric XAML literals 10 and 16\n");
            border = (Border)XamlReader.Load("""<Border xmlns="http://schemas.microsoft.com/winfx/2006/xaml/presentation" Width="400" Height="240" Background="{ThemeResource TestBrush}"/>""");
            border.RequestedTheme = ElementTheme.Light;
            window = new Window { Title = "HLSL C# consumer tests", Content = border };
            window.Activate();
            timer = DispatcherQueue.GetForCurrentThread().CreateTimer(); timer.Interval = TimeSpan.FromMilliseconds(500);
            timer.Tick += Tick; timer.Start();
        }
        catch (Exception e) { Fail(e); }
    }
    private void Tick(DispatcherQueueTimer sender, object args)
    {
        try
        {
            if (phase == 0)
            {
                if (border!.Background is not LiquidGlassBrush b || b.BlurRadius != 10) throw new Exception("Light ThemeResource failed.");
                border.RequestedTheme = ElementTheme.Dark;
            }
            else if (phase == 1)
            {
                if (border!.Background is not LiquidGlassBrush b || b.BlurRadius != 16) throw new Exception("Dark ThemeResource failed.");
                File.AppendAllText("csharp-results.txt", "PASS: Light/Dark ThemeResource\n");
                var compositor = CompositionTarget.GetCompositorForCurrentThread();
                var invalid = HlslEffect.CreateColorTransform("export float4 PSBody(float4 c) { BROKEN }");
                try { HlslComposition.CreateEffectFactory(compositor, invalid); throw new Exception("Bad shader accepted."); }
                catch (System.Runtime.InteropServices.COMException e) when (e.Message.Contains("UserShader.hlsl")) { }
                var schema = new[] { new HlslFloatProperty("Gain", 0.5f, 0, 1) };
                var effect = HlslEffect.CreateColorWithProperties("export float4 PSBody(float4 c){return c*Gain;}", "Input", schema);
                var duplicate = HlslEffect.CreateColorWithProperties("export float4 PSBody(float4 c){return c*Gain;}", "Input", schema);
                if (effect.Id != duplicate.Id) throw new Exception("Deterministic IDs differ.");
                var brush = HlslComposition.CreateEffectFactory(compositor, effect).CreateBrush();
                brush.SetSource("Input", compositor.CreateColorBrush(Microsoft.UI.Colors.Red));
                brush.SetFloat("Gain", 0.75f);
                try { brush.SetFloat("Missing", 0); throw new Exception("Unknown property accepted."); } catch (ArgumentException) { }
                try { brush.SetFloat("Gain", float.NaN); throw new Exception("NaN accepted."); } catch (ArgumentException) { }
                border.Background = HlslComposition.CreateXamlBrush(brush);
                File.AppendAllText("csharp-results.txt", "PASS: generic scalar/source schema, diagnostics, deterministic IDs\n");
            }
            else if (phase == 3)
            {
                var material = new LiquidGlassMaterial(CompositionTarget.GetCompositorForCurrentThread());
                material.RefractionStrength = 20;
                border!.Background = HlslComposition.CreateXamlBrush(material.EffectBrush);
            }
            else if (phase == 5)
            {
                File.AppendAllText("csharp-results.txt", "PASS: native material factory\nALL PASS\n");
                timer!.Stop(); window!.Close();
            }
            ++phase;
        }
        catch (Exception e) { Fail(e); }
    }
    private void Fail(Exception e) { File.AppendAllText("csharp-results.txt", "FAIL " + e + "\n"); timer?.Stop(); window?.Close(); Exit(); }
}
