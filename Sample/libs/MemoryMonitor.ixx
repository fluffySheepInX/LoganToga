module;

#include <Siv3D.hpp>

#if SIV3D_PLATFORM(WINDOWS)
#include <Windows.h>
#include <psapi.h>
#endif

export module MemoryMonitor;
export namespace MemoryMonitor {

	template <size_t MaxSize>
	class RingBuffer {
	public:
		void push(double value) {
			if (buffer.size() == MaxSize) {
				buffer.pop_front();
			}
			buffer.push_back(value);
			if (value > maxValue) {
				maxValue = value;
			}
		}

		const Array<double>& get() const { return buffer; }

		double getMax() const {
			return maxValue > 0 ? maxValue : 1.0; // 安全な最小値
		}

		double getLatest() const {
			return buffer.isEmpty() ? 0.0 : buffer.back();
		}

	private:
		Array<double> buffer;
		double maxValue = 0.0;
	};

	void DrawLineGraph(const Rect& graphArea,
						const Array<double>& values,
						double maxValue,
						const ColorF& color,
						double thickness) {
		if (values.size() < 2) return;

		const double valueCount = static_cast<double>(values.size());
		const double xStep = graphArea.w / (valueCount - 1.0);
		const double yScale = graphArea.h / maxValue;

		LineString lines;
		for (size_t i = 0; i < values.size(); ++i) {
			double x = graphArea.x + xStep * i;
			double y = graphArea.y + graphArea.h - yScale * Clamp(values[i], 0.0, maxValue);
			lines << Vec2{ x, y };
		}
		lines.draw(LineStyle::RoundCap, thickness, color);
	}

	class MemoryMonitor
	{
	public:
		MemoryMonitor() = default;

		void update()
		{
#if SIV3D_PLATFORM(WINDOWS)
			PROCESS_MEMORY_COUNTERS pmc;
			if (!GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc)))
			{
				return;
			}
			update(static_cast<double>(pmc.WorkingSetSize),
				static_cast<double>(pmc.PeakWorkingSetSize),
				static_cast<double>(pmc.PagefileUsage),
				static_cast<double>(pmc.PeakPagefileUsage));
#endif
		}

		void update(const double workingSetSize,
					const double peakWorkingSetSize,
					const double pagefileUsage,
					const double peakPagefileUsage) {
			valuesA.push(workingSetSize / 1024.0);
			valuesB.push(peakWorkingSetSize / 1024.0);
			valuesC.push(pagefileUsage / 1024.0);
			valuesD.push(peakPagefileUsage / 1024.0);
		}

		void draw() const
		{
			draw(Rect{ 40, 40, 600, 400 }, ColorF{ 0.05, 0.05, 0.07, 0.85 });
		}

		void draw(const Rect& graphArea, const ColorF& background) const
		{
			graphArea.draw(background);
			const double maxValue = std::max({ valuesA.getMax(), valuesB.getMax(), valuesC.getMax(), valuesD.getMax() }) * 1.1;
			constexpr int32 tickCount = 5;

			for (int32 i = 0; i <= tickCount; ++i)
			{
				const double t = (tickCount == 0) ? 0.0 : (static_cast<double>(i) / tickCount);
				const double y = graphArea.y + graphArea.h - (graphArea.h * t);
				Line{ graphArea.x, y, graphArea.x + graphArea.w, y }.draw(1, ColorF{ 0.12 });
				const double labelValue = maxValue * t;
				const String label = U"{:.0f} KB"_fmt(labelValue);
				const RectF labelRegion = m_font(label).region();
				m_font(label).draw(graphArea.x - 6 - labelRegion.w, y - (labelRegion.h * 0.5), ColorF{ 0.8 });
			}

			const size_t sampleCount = valuesA.get().size();
			for (int32 i = 0; i <= tickCount; ++i)
			{
				const double t = (tickCount == 0) ? 0.0 : (static_cast<double>(i) / tickCount);
				const double x = graphArea.x + (graphArea.w * t);
				Line{ x, graphArea.y, x, graphArea.y + graphArea.h }.draw(1, ColorF{ 0.10 });
				const size_t index = (sampleCount > 0)
					? static_cast<size_t>(std::round(static_cast<double>(sampleCount - 1) * t))
					: 0;
				m_font(Format(index)).drawAt(x, graphArea.y + graphArea.h + 14, ColorF{ 0.8 });
			}
			m_font(U"samples").draw(graphArea.x + graphArea.w, graphArea.y + graphArea.h + 30, ColorF{ 0.8 });

			graphArea.left().draw(ColorF{ 0.11 });
			graphArea.bottom().draw(ColorF{ 0.11 });
			DrawLineGraph(graphArea, valuesA.get(), maxValue, HSV{ 160, 1.0, 0.9 }, 8);
			DrawLineGraph(graphArea, valuesB.get(), maxValue, HSV{ 220, 1.0, 0.9 }, 6);
			DrawLineGraph(graphArea, valuesC.get(), maxValue, HSV{ 120, 1.0, 0.9 }, 4);
			DrawLineGraph(graphArea, valuesD.get(), maxValue, HSV{ 80, 1.0, 0.9 }, 2);

			const Vec2 legendPos = graphArea.pos.movedBy(12, 10);
			const Array<std::pair<String, ColorF>> legends = {
				{ U"WorkingSet", HSV{ 160, 1.0, 0.9 } },
				{ U"PeakWorkingSet", HSV{ 220, 1.0, 0.9 } },
				{ U"Pagefile", HSV{ 120, 1.0, 0.9 } },
				{ U"PeakPagefile", HSV{ 80, 1.0, 0.9 } },
			};
			const Array<double> latestValues = {
				valuesA.getLatest(),
				valuesB.getLatest(),
				valuesC.getLatest(),
				valuesD.getLatest(),
			};

			for (size_t i = 0; i < legends.size(); ++i)
			{
				const Vec2 rowPos = legendPos.movedBy(0, static_cast<double>(i * 18));
				RectF{ rowPos, 10, 10 }.draw(legends[i].second);
				m_font(U"{}: {:.0f} KB"_fmt(legends[i].first, latestValues[i])).draw(rowPos.movedBy(14, -4), ColorF{ 0.9 });
			}
		}

	private:
		RingBuffer<3000> valuesA, valuesB, valuesC, valuesD;
		Font m_font{ 12 };
	};

	class MemoryMonitorAddon : public IAddon
	{
	public:
		static void Draw()
		{
			EnsureAddon();
		}

	private:
		static constexpr StringView AddonName{ U"MemoryMonitorAddon" };

		static void EnsureAddon()
		{
			if (not Addon::GetAddon<MemoryMonitorAddon>(AddonName))
			{
				Addon::Register<MemoryMonitorAddon>(AddonName);
			}
		}

		bool init() override
		{
			return true;
		}

		bool update() override
		{
#ifndef NDEBUG
			if (KeyF9.down())
			{
				m_visible = !m_visible;
			}
#endif
			if (!m_visible)
			{
				return true;
			}
			m_monitor.update();
			return true;
		}

		void draw() const override
		{
			if (!m_visible)
			{
				return;
			}
			m_monitor.draw(m_graphArea, m_background);
		}

		MemoryMonitor m_monitor;
		Rect m_graphArea{ 40, 40, 600, 400 };
		ColorF m_background{ 0.05, 0.05, 0.07, 0.85 };
		bool m_visible = true;
	};

	inline void Draw()
	{
		MemoryMonitorAddon::Draw();
	}

}
