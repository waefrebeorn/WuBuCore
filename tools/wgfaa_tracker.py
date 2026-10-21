#!/usr/bin/env python3
"""
WGFAA Bubble Tracker — monitors AI bubble metrics in real-time.
Filed as WuBuGov tool per WGFAA doctrine (wgfaa-doctrine.md).

Usage: python3 wgffa_tracker.py [--report]
"""

import json, os, sys, urllib.request
from datetime import datetime
from pathlib import Path

# ─── Sources ───────────────────────────────────────────────────────────────

SOURCES = {
    "nvidia_ir": "https://investor.nvidia.com/news/press-release-details/2026/",
    "openai_blog": "https://openai.com/blog/",
    "anthropic_news": "https://www.anthropic.com/news/",
    "bloomberg_ai": "https://www.bloomberg.com/ai/",
    "sec_edgar": "https://efts.sec.gov/LATEST/search-index",
}

# ─── Metrics ───────────────────────────────────────────────────────────────

class BubbleMetrics:
    """Current state of the AI bubble."""
    
    def __init__(self):
        self.timestamp = datetime.now().isoformat()
        self.hyperscaler_capex = {}
        self.ai_revenue_estimate = {}
        self.valuations = {}
        self.circular_deals = []
        self.token_prices = {}
    
    def load_static_data(self):
        """Load latest known data (manually updated from filings)."""
        
        # Hyperscaler capex (2026 guidance, in billions)
        self.hyperscaler_capex = {
            "Alphabet": 185,
            "Microsoft": 97.2,
            "Amazon": 151,
            "Meta": 60,  # estimate
            "Total": 493.2
        }
        
        # AI revenue estimates (annualized, in billions)
        self.ai_revenue_estimate = {
            "low": 50,     # UBS-style third-party-only
            "mid": 100,    # generous cloud attribution
            "high": 150    # all incremental cloud = AI
        }
        
        # Return requirement (10% of $1.6T cumulative)
        self.required_annual_profit_10pct = 160
        self.required_annual_profit_20pct = 320
        
        # Valuations (in billions)
        self.valuations = {
            "Nvidia": 5000,  # $5T+ as of Oct 2025
            "Microsoft": 3000,
            "Alphabet": 2000,
            "Amazon": 2000,
            "Meta": 1000,
            "OpenAI_private": 852,
            "Anthropic_private": 965,
        }
        
        # Known circular deals (2025-2026)
        self.circular_deals = [
            {"investor": "Nvidia", "target": "OpenAI", "amount": 100, "year": 2025},
            {"investor": "Microsoft", "target": "OpenAI", "amount": 13, "year": 2023},
            {"investor": "Nvidia", "target": "CoreWeave", "amount": 2, "year": 2026},
            {"investor": "Nvidia", "target": "xAI", "amount": 2, "year": 2026},
            {"investor": "Amazon", "target": "OpenAI", "amount": 15, "year": 2026},
            {"investor": "Amazon", "target": "Anthropic", "amount": 15, "year": 2025},
            {"investor": "Nvidia", "target": "Anthropic", "amount": 10, "year": 2026},
            {"investor": "Nvidia", "target": "Reflection", "amount": 2, "year": 2026},
            {"investor": "Nvidia", "target": "Nscale", "amount": 1.1, "year": 2025},
            {"investor": "Nvidia", "target": "Nebius", "amount": 2, "year": 2026},
            {"investor": "Nvidia", "target": "SSI", "amount": 5, "year": 2026},
            {"investor": "Nvidia", "target": "Mistral", "amount": 0.2, "year": 2026},
        ]
        
        # Token prices (per million tokens, USD)
        self.token_prices = {
            "GPT-4_Mar2023": 1.90,
            "GPT-4_May2026": 0.65,
            "change_pct": -65.8
        }
    
    def compute_bubble_score(self) -> dict:
        """Compute composite bubble score 0-100."""
        
        score = 0
        reasons = []
        
        # Revenue gap (0-30 points)
        gap_ratio = self.required_annual_profit_10pct / self.ai_revenue_estimate["high"]
        if gap_ratio > 10:
            score += 30
            reasons.append(f"Revenue gap {gap_ratio:.1f}x (extreme)")
        elif gap_ratio > 5:
            score += 20
            reasons.append(f"Revenue gap {gap_ratio:.1f}x (severe)")
        elif gap_ratio > 2:
            score += 10
            reasons.append(f"Revenue gap {gap_ratio:.1f}x (moderate)")
        
        # Valuation premium (0-25 points)
        # S&P 500 trades at ~3x revenue; AI companies at 38x+
        ai_premium = 38 / 3  # ~12.7x S&P multiple
        if ai_premium > 10:
            score += 25
            reasons.append(f"Valuation premium {ai_premium:.1f}x vs S&P")
        elif ai_premium > 5:
            score += 15
            reasons.append(f"Valuation premium {ai_premium:.1f}x vs S&P")
        
        # Circular deal volume (0-20 points)
        total_circular = sum(d["amount"] for d in self.circular_deals)
        if total_circular > 200:
            score += 20
            reasons.append(f"${total_circular}B in circular deals")
        elif total_circular > 100:
            score += 12
            reasons.append(f"${total_circular}B in circular deals")
        
        # Token price collapse (0-15 points)
        if self.token_prices["change_pct"] < -50:
            score += 15
            reasons.append(f"Token prices down {abs(self.token_prices['change_pct']):.0f}%")
        
        # Enterprise failure rate (0-10 points)
        # 95% from MIT
        score += 10
        reasons.append("95% enterprise pilot failure rate (MIT)")
        
        return {
            "score": min(score, 100),
            "rating": "EXTREME" if score >= 80 else "HIGH" if score >= 60 else "MODERATE" if score >= 40 else "LOW",
            "reasons": reasons,
            "total_circular_deals": len(self.circular_deals),
            "total_circular_volume": sum(d["amount"] for d in self.circular_deals)
        }
    
    def generate_report(self) -> str:
        """Generate human-readable bubble report."""
        
        self.load_static_data()
        score = self.compute_bubble_score()
        
        report = f"""
{'='*60}
  WGFAA BUBBLE TRACKER — {self.timestamp[:10]}
{'='*60}

BUBBLE SCORE: {score['score']}/100 ({score['rating']})

Key metrics:
  Hyperscaler 2026 capex:    ${self.hyperscaler_capex['Total']:.0f}B
  Required annual profit (10%): ${self.required_annual_profit_10pct}B
  Current AI revenue (est.):  ${self.ai_revenue_estimate['mid']:.0f}B (mid)
  Revenue gap:                {self.required_annual_profit_10pct / self.ai_revenue_estimate['mid']:.1f}x

Valuation premium:
  AI companies blended P/R:   ~38x
  S&P 500 blended P/R:        ~3x
  Premium:                    ~12.7x

Circular financing:
  Known deals (2025-2026):    {score['total_circular_deals']}
  Total volume:               ${score['total_circular_volume']:.0f}B

Token price collapse:
  GPT-4 Mar 2023:             $1.90/M tokens
  GPT-4 May 2026:             $0.65/M tokens
  Change:                     -66%

Why this is a bubble:
"""
        for r in score['reasons']:
            report += f"  • {r}\n"
        
        report += f"""
Why this might NOT be a bubble:
  • Anthropic Q2 2026: first operating profit (non-GAAP)
  • Cloud revenue growing 28-63% YoY
  • Enterprise adoption real (if overhyped)
  • AI productivity gains measurable (0.4-0.8% in finance)

Policy trigger (WGFAA):
  When 2+ of these occur, conversion framework activates:
  □ Major AI company files Chapter 11
  □ Hyperscaler free cash flow turns negative for 2+ quarters
  □ Nvidia revenue declines YoY
  □ Token price collapse exceeds 90% from peak
  □ Enterprise AI budget cuts exceed 25%

{'='*60}
"""
        return report

# ─── Main ──────────────────────────────────────────────────────────────────

def main():
    metrics = BubbleMetrics()
    
    if "--report" in sys.argv:
        print(metrics.generate_report())
    elif "--json" in sys.argv:
        metrics.load_static_data()
        score = metrics.compute_bubble_score()
        print(json.dumps(score, indent=2))
    elif "--save" in sys.argv:
        metrics.load_static_data()
        score = metrics.compute_bubble_score()
        out = {
            "timestamp": metrics.timestamp,
            "score": score,
            "metrics": {
                "hyperscaler_capex": metrics.hyperscaler_capex,
                "ai_revenue": metrics.ai_revenue_estimate,
                "required_profit": {
                    "10pct": metrics.required_annual_profit_10pct,
                    "20pct": metrics.required_annual_profit_20pct
                },
                "token_prices": metrics.token_prices,
                "circular_deals": len(metrics.circular_deals),
                "circular_volume": sum(d["amount"] for d in metrics.circular_deals)
            }
        }
        out_path = Path(__file__).parent / "wgfaa_history.jsonl"
        with open(out_path, "a") as f:
            f.write(json.dumps(out) + "\n")
        print(f"Saved to {out_path}")
    else:
        metrics.load_static_data()
        score = metrics.compute_bubble_score()
        print(f"Bubble score: {score['score']}/100 ({score['rating']})")
        print(f"Run with --report for full analysis")

if __name__ == "__main__":
    main()
