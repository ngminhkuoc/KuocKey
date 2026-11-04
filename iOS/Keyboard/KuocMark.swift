import UIKit

/// The small KuocKey mark: the K-roof over a keyhole "o", white on the app icon's indigo tile.
/// Drawn from `design/icon/kuockey-small-roof-keyhole.svg` (same geometry, mark units) so the
/// keyboard extension needs no image asset. Set `layer.cornerRadius` from outside as before.
final class KuocMarkView: UIView {
    private let roof = CAShapeLayer()
    private let o = CAShapeLayer()

    override init(frame: CGRect) {
        super.init(frame: frame)
        backgroundColor = UIColor(red: 0.310, green: 0.275, blue: 0.898, alpha: 1)   // the app icon's indigo in both modes
        layer.cornerCurve = .continuous
        layer.masksToBounds = true
        roof.fillColor = nil
        roof.strokeColor = UIColor.white.cgColor
        roof.lineCap = .round
        roof.lineJoin = .round
        o.fillColor = UIColor.white.cgColor
        o.fillRule = .evenOdd
        layer.addSublayer(roof); layer.addSublayer(o)
        isUserInteractionEnabled = false
    }

    required init?(coder: NSCoder) { fatalError("init(coder:) has not been implemented") }

    override func layoutSubviews() {
        super.layoutSubviews()
        // Mark box in SVG units: x -300...300, y 0...920.25; it fills 52.7% of the tile's height, centred.
        let s = min(bounds.width, bounds.height)
        let k = s * 0.527 / 920.25
        let ox = bounds.midX, oy = bounds.midY - 920.25 * k / 2
        func p(_ x: CGFloat, _ y: CGFloat) -> CGPoint { CGPoint(x: ox + x * k, y: oy + y * k) }

        let r = UIBezierPath()
        r.move(to: p(-255, 45)); r.addLine(to: p(255, 45))
        r.move(to: p(-255, 185.25)); r.addLine(to: p(0, 45)); r.addLine(to: p(255, 185.25))
        roof.path = r.cgPath
        roof.lineWidth = 90 * k

        let c = p(0, 620.25)
        let ring = UIBezierPath(arcCenter: c, radius: 300 * k, startAngle: 0, endAngle: .pi * 2, clockwise: true)
        let hole = UIBezierPath()
        let a = asin(33.111 / 90)   // where the keyhole's stem leaves its round top, at x = ±39
        hole.addArc(withCenter: c, radius: 90 * k, startAngle: .pi - a, endAngle: a, clockwise: true)
        hole.addLine(to: p(66, 770.25))
        hole.addQuadCurve(to: p(48, 788.25), controlPoint: p(66, 788.25))
        hole.addLine(to: p(-48, 788.25))
        hole.addQuadCurve(to: p(-66, 770.25), controlPoint: p(-66, 788.25))
        hole.close()
        ring.append(hole)
        o.path = ring.cgPath
    }
}
